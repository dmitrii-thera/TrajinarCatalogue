#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include "json.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "transport_catalogue.h"
#include "transport_router.h"

namespace Transport::input {

namespace detail {
struct StopDescription {
    std::string name;
    geo::Coordinates coord;
    std::unordered_map<std::string, int> road_distances;
};
struct BusDescription {
    std::string name;
    std::vector<std::string> stops;
    bool is_roundtrip;
};
struct StatDescription {
    int id;
    std::string type;
    std::string name;
    std::string from;
    std::string to;
};

json::Node GetStopStat(
    const TransportCatalogue& catalogue, const detail::StatDescription& desc);
json::Node GetBusStat(
    const TransportCatalogue& catalogue, const detail::StatDescription& desc);

}  // namespace detail

class JsonReader {
  public:
    explicit JsonReader()= default;
    void ParseCommands(std::istream& input);
    void ApplyCommands(TransportCatalogue& catalogue, renderer::MapRenderer& renderer) const;

  private:
    std::vector<detail::BusDescription> bus_requests_;
    std::vector<detail::StopDescription> stop_requests_;
    std::vector<detail::StatDescription> stat_requests_;
    Transport::router::Settings router_settings_;
    
    void ParseBaseRequests(json::Array const& base_requests);
    void ParseStatRequests(json::Array const& stat_requests);
    void ParseRouterSettings(const json::Dict& router_settings);
    void ApplyBaseRequests(TransportCatalogue& catalogue) const;
    void ApplyStatRequests(TransportCatalogue& catalogue, RequestHandler const& request_handler) const;
    void ParseRenderSettings(json::Dict const& render_settings);
    Transport::renderer::detail::Settings render_settings_;
};

}  // namespace Transport::input