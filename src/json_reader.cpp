#include "json_reader.h"
#include "json_builder.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "transport_catalogue.h"

#include <sstream>
#include <stdexcept>
namespace Transport::input {

namespace detail {

json::Node GetBusStat(
    const TransportCatalogue& catalogue, const detail::StatDescription& desc) {
    using namespace std::string_literals;
    // json::Dict result{{"request_id"s, json::Node(desc.id)}};
    if (!catalogue.FindBus(desc.name)) {
        // result["error_message"s] = "not found"s;
        // return result;
        return json::Builder{}
        .StartDict()
        .Key("request_id"s)
        .Value(desc.id)
        .Key("error_message"s)
        .Value("not found"s)
        .EndDict().Build();
    }
    auto routes = catalogue.GetBusInfo(desc.name);



    return json::Builder{}
        .StartDict()
        .Key("request_id"s)
        .Value(desc.id)
        .Key("curvature"s)
        .Value(routes.unevenness)
        .Key("route_length"s)
        .Value(routes.route_length)
        .Key("stop_count"s)
        .Value(static_cast<int>(routes.count_stop))
        .Key("unique_stop_count"s)
        .Value(static_cast<int>(routes.count_UStop))
        .EndDict()
        .Build();
}

json::Node GetStopStat(
    const TransportCatalogue& catalogue, const detail::StatDescription& desc) {
    using namespace std::string_literals;

    if (!catalogue.FindStop(desc.name)) {
        return json::Builder{}
        .StartDict()
        .Key("request_id"s)
        .Value(desc.id)
        .Key("error_message"s)
        .Value("not found"s)
        .EndDict().Build();

    }
    auto routes = catalogue.GetStopInfo(desc.name);
    json::Array buses;
    for (const auto* p : routes) {
        buses.emplace_back(json::Node(p->name));
    }

    return json::Builder{}
        .StartDict()
        .Key("request_id"s)
        .Value(desc.id)
        .Key("buses"s)
        .Value(std::move(buses))
        .EndDict().Build();

}

 renderer::detail::Color ParseColor(json::Node const& node) {
    using namespace std::string_literals;

    if (node.IsString()) {
        return node.AsString();
    }
    if (node.IsArray()) {
        if (node.AsArray().size() == 3) {
            return svg::Rgb{static_cast<uint8_t>(node.AsArray()[0].AsInt()),
                static_cast<uint8_t>(node.AsArray()[1].AsInt()),
                static_cast<uint8_t>(node.AsArray()[2].AsInt())};
        }
        return svg::Rgba{static_cast<uint8_t>(node.AsArray()[0].AsInt()),
            static_cast<uint8_t>(node.AsArray()[1].AsInt()),
            static_cast<uint8_t>(node.AsArray()[2].AsInt()),
            node.AsArray()[3].AsDouble()};
    }
    return std::monostate{};
 }

json::Node GetMap(
    RequestHandler const& request_handler, StatDescription const& desc) {
    using namespace std::string_literals;

    std::ostringstream oss;
    request_handler.RenderMap().Render(oss);

    return json::Builder{}
        .StartDict()
        .Key("request_id"s)
        .Value(desc.id)
        .Key("map"s)
        .Value(oss.str())
        .EndDict().Build();
}

json::Node GetRoute(RequestHandler const& request_handler, StatDescription const& desc)
{
  using namespace std::string_literals;

auto route = request_handler.BuildRoute(desc.from, desc.to);

if(!route){
return json::Builder{}.StartDict().Key("request_id"s).Value(desc.id).Key("error_message"s).Value("not found"s).EndDict().Build();
}

json::Array items;
double total_time{0.};
items.reserve(route->edges.size());
for(auto const& edge_id : route->edges) {
    if (request_handler.IsWaitEdge(edge_id)) {
        auto info = request_handler.AsWaitEdge(edge_id);
        items.emplace_back(json::Node(json::Builder{}
                                          .StartDict()
                                          .Key("stop_name"s)
                                          .Value(std::string(info.stop_name))
                                          .Key("time"s)
                                          .Value(info.time)
                                          .Key("type"s).Value("Wait"s)
                                          .EndDict().Build()));
                                          total_time += info.time;
    }else{            auto info = request_handler.AsBusEdge(edge_id);
            items.emplace_back( json::Node(json::Builder{} .StartDict()
                .Key("bus_name"s).Value(std::string(info.bus_name))
                .Key("span_count"s).Value(static_cast<int>(info.span_count))
                .Key("time"s).Value(info.time)
                .Key("type"s).Value("Bus"s)
                .EndDict() .Build()));
            total_time += info.time;
    }
}
return json::Builder{}
    .StartDict()
    .Key("request_id"s)
    .Value(desc.id)
    .Key("total_time"s)
    .Value(total_time)
    .Key("items"s)
    .Value(std::move(items))
    .EndDict()
    .Build();
}


}  // namespace detail

void JsonReader::ParseBaseRequests(json::Array const& base_requests) {
    using namespace std::string_literals;
    try {
        for (const json::Node& node : base_requests) {
            auto req_map = node.AsDict();
            auto type_it = req_map.find("type"s);
            if (type_it != req_map.end()) {
                std::string type = type_it->second.AsString();

                if (type == "Stop"s) {
                    detail::StopDescription stop{};
                    stop.name = req_map.at("name"s).AsString();
                    stop.coord.lat = req_map.at("latitude"s).AsDouble();
                    stop.coord.lng = req_map.at("longitude"s).AsDouble();
                    if (auto dist_it = req_map.find("road_distances"s);
                        dist_it != req_map.end()) {
                        for (const auto& [stop_name, distance] :
                            dist_it->second.AsDict()) {
                            stop.road_distances[stop_name] = distance.AsInt();
                        }
                    }
                    stop_requests_.push_back(std::move(stop));
                } else if (type == "Bus"s) {
                    detail::BusDescription bus{};
                    bus.name = req_map.at("name"s).AsString();
                    bus.is_roundtrip = req_map.at("is_roundtrip"s).AsBool();
                    std::vector<std::string> result;
                    for (const auto& node : req_map.at("stops"s).AsArray()) {
                        result.emplace_back(node.AsString());
                    }
                    bus.stops.insert(
                        bus.stops.begin(), result.begin(), result.end());
                    bus_requests_.push_back(std::move(bus));
                }
            }
        }
    } catch (const std::exception& ex) {
        std::cerr << "Error parsing base request: " << ex.what() << std::endl;
    }
}

void JsonReader::ParseStatRequests(json::Array const& stat_requests) {
    using namespace std::string_literals;
    for (const json::Node& node : stat_requests) {
        const auto& map = node.AsDict();
        detail::StatDescription stat{};

        try {
            stat.id = map.at("id"s).AsInt();
            stat.type = map.at("type"s).AsString();
            if (stat.type == "Bus"s || stat.type == "Stop"s) {
                stat.name = map.at("name"s).AsString();
            }
            if (!(stat.type == "Route")) {
                stat.from = map.at("from"s).AsString();
                stat.to = map.at("to"s).AsString();
            }
            stat_requests_.push_back(std::move(stat));
        } catch (const std::exception& ex) {
            std::cerr << "Error parsing stat request: " << ex.what()
                      << std::endl;
        }
    }
}

void JsonReader::ParseRenderSettings(json::Dict const& render_settings) {
    using namespace std::string_literals;
    Transport::renderer::detail::Settings settings;
    settings.width = render_settings.at("width"s).AsDouble();
    settings.height = render_settings.at("height"s).AsDouble();
    settings.padding = render_settings.at("padding"s).AsDouble();
    settings.line_width = render_settings.at("line_width"s).AsDouble();
    settings.stop_radius = render_settings.at("stop_radius"s).AsDouble();

    settings.bus_label_font_size =
        render_settings.at("bus_label_font_size"s).AsInt();

    auto bus_label_offset = render_settings.at("bus_label_offset"s).AsArray();
    settings.bus_label_offset = {
        bus_label_offset[0].AsDouble(), bus_label_offset[1].AsDouble()};

    settings.stop_label_font_size =
        render_settings.at("stop_label_font_size"s).AsInt();

    auto stop_label_offset = render_settings.at("stop_label_offset"s).AsArray();
    settings.stop_label_offset = {
        stop_label_offset[0].AsDouble(), stop_label_offset[1].AsDouble()};

    auto underlayer_color_node = render_settings.at("underlayer_color"s);
    settings.underlayer_color =
        detail::ParseColor(underlayer_color_node);

    settings.underlayer_width =
        render_settings.at("underlayer_width"s).AsDouble();

    auto arrayColor = render_settings.at("color_palette"s).AsArray();
    for (auto& s : arrayColor) {
        settings.color_palette.emplace_back(detail::ParseColor(s));
    }

    render_settings_ = settings;
}
 void JsonReader::ParseRouterSettings(const json::Dict& router_settings){
    using namespace std::string_literals;
    try{
        router_settings_.bus_wait_time = router_settings.at("bus_wait_time"s).AsInt();
        router_settings_.bus_velocity = router_settings.at("bus_velocity"s).AsInt();
    }catch (std::out_of_range const& e){
        std::cerr << e.what() << std::endl;
        // throw std::logic_error(__func__ + "failed"s);
    }
 }






void JsonReader::ParseCommands(std::istream& input) {
    using namespace std::string_literals;
    auto root = json::Load(input).GetRoot().AsDict();
    if (auto it = root.find("base_requests"s); it != root.end()) {
        // ParseBaseRequests(root.at("base_requests"s).AsArray());
        ParseBaseRequests(it->second.AsArray());
    }
    if (auto it = root.find("stat_requests"s); it != root.end()) {
        // ParseStatRequests(root.at("stat_requests"s).AsArray());
        ParseStatRequests(it->second.AsArray());
    }

    if (auto it = root.find("render_settings"s); it != root.end()) {
        ParseRenderSettings(it->second.AsDict());
    }

    if (auto it = root.find("routing_settings"s); it != root.end()) {
        ParseRouterSettings(it->second.AsDict());
    }
}

void JsonReader::ApplyBaseRequests(TransportCatalogue& catalogue) const{
     for (auto const& stopReq : stop_requests_) {
        Stop temp = {stopReq.name, stopReq.coord};
        catalogue.AddStop(temp);
    }
    for (auto const& desc : stop_requests_) {
        auto const* stop = catalogue.FindStop(desc.name);
        for (const auto& [to_name, distance] : desc.road_distances) {
            catalogue.SetDistance(stop, catalogue.FindStop(to_name), distance);
        }
    }
    for (auto const& busReq : bus_requests_) {
        Bus bus{};
        bus.name = busReq.name;
        bus.is_circle_ = busReq.is_roundtrip;
        for (auto const& stopname : busReq.stops) {
            bus.route.push_back(catalogue.FindStop(stopname));
        }
        catalogue.AddBus(bus);
    }
}

void JsonReader::ApplyStatRequests(TransportCatalogue& catalogue,
    RequestHandler const& request_handler) const {
    using namespace std::string_literals;
    if (!stat_requests_.empty()) {
        json::Array node;
        for (auto const& desc : stat_requests_) {
            if (desc.type == "Stop"s) {
                node.emplace_back(detail::GetStopStat(catalogue, desc));
            }
            if (desc.type == "Bus"s) {
                node.emplace_back(detail::GetBusStat(catalogue, desc));
            }
            if (desc.type == "Map"s) {
            node.emplace_back(detail::GetMap(request_handler, desc));
            }
            if (desc.type == "Route"s) {
                node.emplace_back(detail::GetRoute(request_handler, desc));
            }
        }

        json::Print(json::Document(node), std::cout);
    }
}

void JsonReader::ApplyCommands(TransportCatalogue& catalogue, renderer::MapRenderer& render) const {

    //* apply base request bus stop //
    ApplyBaseRequests(catalogue);

    /* apply render requests setting */
    render.SetSettings(render_settings_);
   Transport::TransportRouter router(router_settings_, catalogue);
    /* process stat request*/
    Transport::RequestHandler request_handler(catalogue, render, router);
    ApplyStatRequests(catalogue, request_handler);
}




}  // namespace Transport::input
