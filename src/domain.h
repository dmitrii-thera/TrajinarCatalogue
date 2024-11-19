#pragma once
#include "geo.h"

#include <string>
#include <vector>

namespace Transport {
struct Stop {
  std::string name;
  geo::Coordinates coordinate;
};

struct Bus {
  std::string name;
  std::vector<const Stop*> route;
  bool is_circle_;
};

struct BusInfo {
  size_t count_stop;
  size_t count_UStop;
  double route_length;
  double unevenness = 0.0;
};
}  // namespace Transport