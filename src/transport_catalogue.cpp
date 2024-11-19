#include "transport_catalogue.h"

#include <unordered_map>
#include <unordered_set>


namespace Transport {
void TransportCatalogue::AddStop(const Stop& stop) {
  stops_.push_back(stop);
  stopname_to_stop_[stops_.back().name] = &stops_.back();
}

const Stop* TransportCatalogue::FindStop(std::string_view name) const {
  const auto it = stopname_to_stop_.find(name);
  if (it != stopname_to_stop_.end()) {
    return it->second;
  }
  return nullptr;
}

void TransportCatalogue::AddBus(const Bus& bus) {
  buses_.push_back(bus);
  busname_to_bus_[buses_.back().name] = &buses_.back();
  for (auto route : buses_.back().route) {
    stop_to_routes_[route->name].insert(&buses_.back());
  }
}

const Bus* TransportCatalogue::FindBus(std::string_view name) const {
  const auto it = busname_to_bus_.find(name);
  if (it != busname_to_bus_.end()) {
    return it->second;
  }
  return nullptr;
}

BusInfo TransportCatalogue::GetBusInfo(std::string_view name) const {
  BusInfo temp{};
  using namespace std::string_literals;
  const auto* bus = FindBus(name);
  if (!bus) {
    return {};
  }

  std::unordered_set<const Stop*> busUnique(bus->route.begin(),
                                            bus->route.end());
  temp.count_UStop = busUnique.size();

  std::vector<Stop const*> route;
  route.insert(route.begin(), bus->route.begin(), bus->route.end());
  if (!bus->is_circle_) {
    route.insert(route.end(), std::next(bus->route.rbegin()),
                 bus->route.rend());
  }

  temp.count_stop = route.size();

  // double route_length{};
  double geo_length{};

  for (auto it{route.begin()}; it != prev(route.end()); ++it) {
    auto from = (*it)->coordinate;
    auto to = (*next(it))->coordinate;
    geo_length += geo::ComputeDistance(from, to);
    auto pair = std::pair(*it, *std::next(it));
    auto pair_it = distances_.find(pair);
    if (pair_it != distances_.end()) {
      temp.route_length += distances_.at(pair);
    } else {
      pair = std::pair(*next(it), *it);
      pair_it = distances_.find(pair);
      if (pair_it != distances_.end()) {
        temp.route_length += distances_.at(pair);
      }
    }
  }
  temp.unevenness = temp.route_length / geo_length;
  return temp;
}

std::set<const Bus*, TransportCatalogue::BusComparator>
TransportCatalogue::GetStopInfo(std::string_view name) const {
    auto it = stop_to_routes_.find(name);
    if (it != stop_to_routes_.end()) {
        return it->second;
    }
    return {};
}

void TransportCatalogue::SetDistance(Stop const* lhs, Stop const* rhs,
                                     const int distance) {
  distances_[{lhs, rhs}] = distance;
}

std::deque<Bus> const& TransportCatalogue::GetRoutes() const { return buses_; }

std::deque<Stop> const& TransportCatalogue::GetStops() const

{
  return stops_;
}

int TransportCatalogue::GetDistance(const Stop* from, const Stop* to) const {
  if (distances_.count({from, to})) {
    return distances_.at({from, to});
  } else if (distances_.count({to, from})) {
    return distances_.at({to, from});
  } else {
    return 0;
  }
}

std::pair<int, double> TransportCatalogue::GetLength(const Bus* bus) const {
  int total_distance = 0;
  double total_length = 0.0;

  for (size_t i = 1; i < bus->route.size(); ++i) {
    total_distance += GetDistance(bus->route[i - 1], bus->route[i]);
  }

  if (bus->is_circle_) {
    for (size_t i = 1; i < bus->route.size(); ++i) {
      total_length += geo::ComputeDistance(bus->route[i - 1]->coordinate,
                                           bus->route[i]->coordinate);
    }
  } else {
    for (size_t i = bus->route.size() - 1; i > 0; --i) {
      total_length += geo::ComputeDistance(bus->route[i]->coordinate,
                                           bus->route[i - 1]->coordinate);
    }
  }

  return {total_distance, total_length};
}
}  // namespace Transport
