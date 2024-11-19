#pragma once
#include "domain.h"
#include "geo.h"
#include "svg.h"

#include <algorithm>
#include <utility>

/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршртутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */

namespace Transport::renderer {
namespace detail {
using Color = svg::Color;
using Point = svg::Point;
using Rgba = svg::Rgba;
using Rgb = svg::Rgb;

struct Settings {
  Settings() = default;

  double width{};
  double height{};

  double stop_radius{};

  double padding{};

  double line_width{};

  Point stop_label_offset;

  int stop_label_font_size{};

  int bus_label_font_size{};

  double underlayer_width{};

  Point bus_label_offset;

  Color underlayer_color;

  std::vector<Color> color_palette;
};
}  // namespace detail

using BusPtr = Bus const*;
using StopPtr = Stop const*;
using Buses = std::vector<BusPtr>;
using Stops = std::vector<StopPtr>;

class SphereProjector;

//detail::Color ParseColor(json::Node const& node);

class MapRenderer {
 public:
  MapRenderer() = default;

  detail::Settings GetSettings() const { return settings_; }

  svg::Document RenderMap(Buses const& routes, Stops const& stops) const;

  void SetSettings(const detail::Settings& settings) { settings_ = settings; }

 private:
  detail::Settings settings_;
  void AddRoutes(svg::Document& doc, Buses const& routes,
                 SphereProjector const& proj) const;
  void AddRouteNames(svg::Document& doc, Buses const& routes,
                     SphereProjector const& proj) const;
  void AddStopShapes(svg::Document& doc, Stops const& stops,
                     SphereProjector const& proj) const;
  void AddStopNames(svg::Document& doc, Stops const& stops,
                    SphereProjector const& proj) const;
};

bool IsZero(double value);

class SphereProjector {
public:
    // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                    double max_width, double max_height, double padding)
        : padding_(padding) //
    {
        // Если точки поверхности сферы не заданы, вычислять нечего
        if (points_begin == points_end) {
            return;
        }

        // Находим точки с минимальной и максимальной долготой
        const auto [left_it, right_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
        min_lon_ = left_it->lng;
        const double max_lon = right_it->lng;

        // Находим точки с минимальной и максимальной широтой
        const auto [bottom_it, top_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
        const double min_lat = bottom_it->lat;
        max_lat_ = top_it->lat;

        // Вычисляем коэффициент масштабирования вдоль координаты x
        std::optional<double> width_zoom;
        if (!IsZero(max_lon - min_lon_)) {
            width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
        }

        // Вычисляем коэффициент масштабирования вдоль координаты y
        std::optional<double> height_zoom;
        if (!IsZero(max_lat_ - min_lat)) {
            height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
        }

        if (width_zoom && height_zoom) {
            // Коэффициенты масштабирования по ширине и высоте ненулевые,
            // берём минимальный из них
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        } else if (width_zoom) {
            // Коэффициент масштабирования по ширине ненулевой, используем его
            zoom_coeff_ = *width_zoom;
        } else if (height_zoom) {
            // Коэффициент масштабирования по высоте ненулевой, используем его
            zoom_coeff_ = *height_zoom;
        }
    }

    // Проецирует широту и долготу в координаты внутри SVG-изображения
    svg::Point operator()(Transport::geo::Coordinates coords) const {
        return {
            (coords.lng - min_lon_) * zoom_coeff_ + padding_,
            (max_lat_ - coords.lat) * zoom_coeff_ + padding_
        };
    }

private:
    double padding_;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;
};
}  // namespace Transport::renderer