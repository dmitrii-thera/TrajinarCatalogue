#include "transport_router.h"
namespace Transport {

std::optional<typename graph::Router<double>::RouteInfo>
TransportRouter::BuildRoute(std::string_view from, std::string_view to) const {
    return router_->BuildRoute(FindVertexId(from), FindVertexId(to));
};

bool TransportRouter::EdgeInfo::IsWaitEdge() const {
    return std::holds_alternative<router::WaitEdgeInfo>(*this);
}
bool TransportRouter::EdgeInfo::IsBusEdge() const {
    return std::holds_alternative<router::BusEdgeInfo>(*this);
}
router::WaitEdgeInfo TransportRouter::EdgeInfo::AsWaitEdge() const {
    using namespace std::literals;
    if (!IsWaitEdge()) {
        throw std::logic_error("Not a WaitEdge"s);
    }
    return std::get<router::WaitEdgeInfo>(*this);
}
router::BusEdgeInfo TransportRouter::EdgeInfo::AsBusEdge() const {
    using namespace std::literals;
    if (!IsBusEdge()) {
        throw std::logic_error("Not a BusEdge"s);
    }
    return std::get<router::BusEdgeInfo>(*this);
}

void TransportRouter::BuildRouter(
    Transport::TransportCatalogue const& catalogue) {
        graph::DirectedWeightedGraph<double> graph;
        router_ = std::make_unique<graph::Router<double>>(graph);
};

graph::VertexId const& TransportRouter::FindVertexId(std::string_view stop) const {
    using namespace std::string_literals;
    auto it = stop_to_vid_.find(stop);
    if (it == stop_to_vid_.end()) {
        throw std::out_of_range(__func__ + " VertexId not found"s);
    }
    return it->second;
}


}  // namespace Transport
