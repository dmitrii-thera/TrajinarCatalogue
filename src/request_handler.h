#pragma once
#include "map_renderer.h"
#include "svg.h"
#include "transport_catalogue.h"
#include "transport_router.h"

// #include <unordered_set>

namespace Transport {

using BusStat = BusInfo;
using BusPtr = Bus const*;
using StopPtr = Stop const*;

class RequestHandler {
  public:
    RequestHandler(const TransportCatalogue& db,
        const renderer::MapRenderer& render, const Transport::TransportRouter& router);

    // Возвращает информацию о маршруте (запрос Bus)
    // std::optional<BusStat> GetBusStat(const std::string_view& bus_name)
    // const;

    // Возвращает маршруты, проходящие через
    // const std::unordered_set<BusPtr>* GetBusesByStop(const std::string_view&
    // stop_name) const;

    svg::Document RenderMap() const;

    std::optional<typename graph::Router<double>::RouteInfo> BuildRoute(
        std::string_view from, std::string_view to) const;
    
    bool IsWaitEdge(graph::EdgeId edge_id) const;
    router::WaitEdgeInfo AsWaitEdge(graph::EdgeId edge_id) const;
    router::BusEdgeInfo AsBusEdge(graph::EdgeId edge_id) const;

  private:
    // RequestHandler использует агрегацию объектов "Транспортный Справочник"
    /// и "Визуализатор Карты"

    const TransportCatalogue& db_;
    const renderer::MapRenderer& renderer_;
    const TransportRouter& router_;
};

}  // namespace Transport