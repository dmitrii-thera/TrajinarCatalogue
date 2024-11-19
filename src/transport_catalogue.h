#pragma once
#include "domain.h"

#include <deque>
#include <set>
#include <string_view>
#include <unordered_map>


namespace Transport {


class TransportCatalogue {
 public:
    struct BusComparator {
    bool operator()(const Bus* lhs, const Bus* rhs) const {
        return lhs->name < rhs->name;
    }
    };
    struct Hasher {
    size_t operator()(const Stop* stop) const noexcept {
        return ((hasher_db(stop->coordinate.lat) * 37) +
                (hasher_db(stop->coordinate.lng) * (37 * 37)));
    }

    size_t operator()(
        const std::pair<const Stop*, const Stop*> pair_stops) const noexcept {
        auto hash_1 = static_cast<const void*>(pair_stops.first);
        auto hash_2 = static_cast<const void*>(pair_stops.second);

        return ((hasher_ptr(hash_1) * 37) + (hasher_ptr(hash_2) * (37 * 37)));
    }

    private:
    std::hash<double> hasher_db;
    std::hash<const void*> hasher_ptr;
    };

  void AddStop(const Stop& stop);
  const Stop* FindStop(std::string_view name) const;

  void AddBus(const Bus& bus);
  const Bus* FindBus(std::string_view name) const;
  BusInfo GetBusInfo(
      std::string_view name) const;
  std::set<const Bus*, BusComparator> GetStopInfo(
      std::string_view name) const;
  void SetDistance(Stop const* lhs, Stop const* rhs, const int distance);
  std::deque<Stop> const& GetStops() const;
  std::deque<Bus> const& GetRoutes() const;

  int GetDistance(const Stop* from, const Stop* to) const;

  std::pair<int, double> GetLength(const Bus* bus) const;

 private:
  std::deque<Stop> stops_;

  std::unordered_map<std::string_view, const Stop*> stopname_to_stop_;
  std::unordered_map<std::string_view, std::set<const Bus*, BusComparator>>
      stop_to_routes_;
  std::vector<Stop*> routes_;
  std::deque<Bus> buses_;
  std::unordered_map<std::string_view, const Bus*> busname_to_bus_;
  std::unordered_map<std::pair<const Stop*, const Stop*>, int, Hasher>
      distances_;
};
}  // namespace Transport
