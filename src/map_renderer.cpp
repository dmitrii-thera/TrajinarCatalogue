#include "map_renderer.h"

/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршртутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */

namespace Transport::renderer {

inline const double EPSILON = 1e-6;

bool IsZero(double value) { return std::abs(value) < EPSILON; }

svg::Document MapRenderer::RenderMap(Buses const& routes,
                                     Stops const& stops) const {
  svg::Document doc;

  /* set projector */
  std::vector<geo::Coordinates> geo_coords{};
  for (const auto& stop_ptr : stops) {
    geo_coords.push_back(stop_ptr->coordinate);
  }
  const renderer::SphereProjector proj{geo_coords.begin(), geo_coords.end(),
                                       settings_.width, settings_.height,
                                       settings_.padding};

  AddRoutes(doc, routes, proj);
  AddRouteNames(doc, routes, proj);
  AddStopShapes(doc, stops, proj);
  AddStopNames(doc, stops, proj);

  return doc;
}

void MapRenderer::AddRoutes(
    svg::Document& doc, Buses const& routes,
    renderer::SphereProjector const& proj) const {  // <-- NOTE Buses
  using namespace std::string_literals;
  size_t color_index{};
  for (auto const* const it : routes) {  // <-- NOTE is pointers bus_ptr
    svg::Polyline polyline;
    for (auto const* const stop : it->route) {  //
      const svg::Point screen_coord = proj(stop->coordinate);
      polyline.AddPoint(screen_coord);
    }
    if (!it->is_circle_) {
      for (auto stop_it{std::next(it->route.rbegin())};
           stop_it != it->route.rend(); ++stop_it) {
        const svg::Point screen_coord = proj((*stop_it)->coordinate);
        polyline.AddPoint(screen_coord);
      }
    }
    doc.Add(polyline.SetFillColor("none"s)
                .SetStrokeColor(
                    settings_.color_palette[color_index])  // NOTE settings_
                .SetStrokeWidth(settings_.line_width)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND));
    color_index =
        ++color_index == settings_.color_palette.size() ? 0 : color_index;
  }
}
void MapRenderer::AddRouteNames(
    svg::Document& doc, Buses const& routes,
    renderer::SphereProjector const& proj) const {  // NOTE Buses
  using namespace std::string_literals;
  size_t color_index{};
  for (auto const* const it : routes) {  // NOTE is pointer now 'bus_ptr'
    svg::Text text(svg::Text()
                       .SetOffset(settings_.bus_label_offset)  // NOTE settings_
                       .SetFontSize(settings_.bus_label_font_size)
                       .SetFontFamily("Verdana"s)
                       .SetFillColor(settings_.color_palette[color_index])
                       .SetFontWeight("bold"s));
    svg::Text underlayer(svg::Text()
                             .SetOffset(settings_.bus_label_offset)
                             .SetFontSize(settings_.bus_label_font_size)
                             .SetFontFamily("Verdana"s)
                             .SetFillColor(settings_.underlayer_color)
                             .SetStrokeColor(settings_.underlayer_color)
                             .SetStrokeWidth(settings_.underlayer_width)
                             .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                             .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
                             .SetFontWeight("bold"s));
    Stop const* stop_begin = *(it->route.begin());
    doc.Add(
        underlayer.SetPosition(proj(stop_begin->coordinate)).SetData(it->name));
    doc.Add(text.SetPosition(proj(stop_begin->coordinate)).SetData(it->name));
    if (!it->is_circle_) {
      Stop const* stop_end = *(it->route.end() - 1);
      if (stop_begin->name != stop_end->name) {
        doc.Add(underlayer.SetPosition(proj(stop_end->coordinate))
                    .SetData(it->name));
        doc.Add(text.SetPosition(proj(stop_end->coordinate)).SetData(it->name));
      }
    }
    color_index =
        ++color_index == settings_.color_palette.size() ? 0 : color_index;
  }
}
void MapRenderer::AddStopShapes(
    svg::Document& doc, Stops const& stops,
    renderer::SphereProjector const& proj) const {  // NOTE Stops
  using namespace std::string_literals;
  svg::Circle circle;
  for (auto const* const it : stops) {  // NOTE is stop_ptr now
    const svg::Point screen_coord = proj(it->coordinate);
    doc.Add(circle.SetCenter(screen_coord)
                .SetRadius(settings_.stop_radius)
                .SetFillColor("white"s));
  }
}
void MapRenderer::AddStopNames(
    svg::Document& doc, Stops const& stops,
    renderer::SphereProjector const& proj) const {  // NOTE Stops
  using namespace std::string_literals;
  svg::Text text(svg::Text()
                     .SetOffset(settings_.stop_label_offset)
                     .SetFontSize(settings_.stop_label_font_size)
                     .SetFontFamily("Verdana"s)
                     .SetFillColor("black"s));
  svg::Text underlayer(svg::Text()
                           .SetOffset(settings_.stop_label_offset)
                           .SetFontSize(settings_.stop_label_font_size)
                           .SetFontFamily("Verdana"s)
                           .SetFillColor(settings_.underlayer_color)
                           .SetStrokeColor(settings_.underlayer_color)
                           .SetStrokeWidth(settings_.underlayer_width)
                           .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                           .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND));
  for (auto const* const it : stops) {  // NOTE is stop_ptr now
    auto const screen_coord = proj(it->coordinate);
    doc.Add(underlayer.SetPosition(screen_coord).SetData(it->name));
    doc.Add(text.SetPosition(screen_coord).SetData(it->name));
  }
}

}  // namespace Transport::renderer