#include "request_handler.h"

#include <string_view>
namespace Transport {

RequestHandler::RequestHandler(TransportCatalogue const& db,
    renderer::MapRenderer const& renderer,
    Transport::TransportRouter const& router)
    : db_(db), renderer_(renderer), router_(router) {};

// std::optional<BusStat>
// RequestHandler::GetBusStat(const std::string_view& bus_name) const;

// const std::unordered_set<BusPtr>*
// RequestHandler::GetBusesByStop(const std::string_view& stop_name) const;

svg::Document RequestHandler::RenderMap() const {
    using namespace std::string_literals;

    std::set<std::string_view> route_names_sorted;
    std::set<std::string_view> stop_names_sorted;
    for (auto const& bus : db_.GetRoutes()) {
        if (bus.route.empty()) {
            continue;
        }
        route_names_sorted.insert(bus.name);
        for (auto const* stop : bus.route) {
            stop_names_sorted.insert(stop->name);
        }
    }

    std::vector<BusPtr> routes;
    for (auto const& bus_name : route_names_sorted) {
        auto const* const bus_ptr = db_.FindBus(bus_name);
        routes.push_back(bus_ptr);
    }
    std::vector<StopPtr> stops;
    for (auto const& stop_name : stop_names_sorted) {
        auto const* const stop_ptr = db_.FindStop(stop_name);
        stops.push_back(stop_ptr);
    }

    return renderer_.RenderMap(routes, stops);
};

 // namespace Transport

std::optional<typename graph::Router<double>::RouteInfo>
RequestHandler::BuildRoute(std::string_view from, std::string_view to) const {
        return router_.BuildRoute(from, to);
}


bool RequestHandler::IsWaitEdge(graph::EdgeId edge_id) const {
         return router_.IsWaitEdge(edge_id);
 }
router::WaitEdgeInfo RequestHandler::AsWaitEdge(graph::EdgeId edge_id) const {
    return router_.AsWaitEdge(edge_id);
}
router::BusEdgeInfo RequestHandler::AsBusEdge(graph::EdgeId edge_id) const {
    return router_.AsBusEdge(edge_id);
}
}