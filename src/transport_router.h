#pragma once
#include "graph.h"
#include "router.h"
#include "transport_catalogue.h"

#include <memory>
#include <variant>

namespace Transport { namespace router {

struct Settings {
    int bus_wait_time{}; // minutes
    double bus_velocity{}; // kph
};

struct WaitEdgeInfo {
    std::string_view stop_name;
    double time{0.};
};

struct BusEdgeInfo {
    std::string_view bus_name;
    size_t span_count{};
    double time{0.};
};

using EdgeInfo = std::variant<WaitEdgeInfo, BusEdgeInfo>;

}  // namespace router


class TransportRouter {
  public:
    explicit TransportRouter(
        router::Settings settings, TransportCatalogue const& catalogue)
        : settings_(settings) {
        BuildRouter(catalogue);
    };

    std::optional<typename graph::Router<double>::RouteInfo> BuildRoute(
        std::string_view from, std::string_view to) const;

    bool IsWaitEdge(graph::EdgeId edge_id) const {
        return eid_to_info_.at(edge_id).IsWaitEdge();
    }

    router::WaitEdgeInfo AsWaitEdge(graph::EdgeId edge_id) const {
        return eid_to_info_.at(edge_id).AsWaitEdge();
    }

    router::BusEdgeInfo AsBusEdge(graph::EdgeId edge_id) const {
        return eid_to_info_.at(edge_id).AsBusEdge();
    }

  private:
    router::Settings settings_{};
    graph::DirectedWeightedGraph<double> graph_;
    std::unique_ptr<graph::Router<double>> router_;
    std::unordered_map<std::string_view, graph::VertexId> stop_to_vid_;
    void BuildRouter(TransportCatalogue const& catalogue);

    class EdgeInfo final : router::EdgeInfo {
      public:
        using variant::variant;
        using Value = variant;
        explicit EdgeInfo(Value value) : variant(value) {}
        bool IsWaitEdge() const;
        bool IsBusEdge() const;
        router::WaitEdgeInfo AsWaitEdge() const;
        router::BusEdgeInfo AsBusEdge() const;
    };

    std::unordered_map<graph::EdgeId, EdgeInfo> eid_to_info_;
    graph::VertexId const& FindVertexId(std::string_view stop) const;

};

} // namespace Transport