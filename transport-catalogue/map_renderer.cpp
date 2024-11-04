#include "map_renderer.h"

#include <cassert>

using namespace svg;
using namespace std::literals;

namespace renderer {

bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}
    
MapRenderer::MapRenderer(const domain::RenderSettings& settings, const transport::TransportCatalogue& db) : settings_(settings), db_(db) {
    //
}
    
const svg::Document MapRenderer::Render() const {
    const auto& allstops = db_.GetStops();
    SphereProjector projector(allstops.cbegin(), allstops.cend(), settings_.width, settings_.height, settings_.padding);
    svg::Document doc;
    RenderRoutes(projector, doc);
    RenderStops(projector, doc);
    return doc;
}
    
void MapRenderer::RenderRoutes(const SphereProjector& projector, svg::Document& doc) const {
    std::vector<std::string_view> buses = db_.GetBuses();
    std::sort(buses.begin(), buses.end());
    auto color_it = settings_.color_palette.begin();
    for (const auto& bus : buses) {
        const auto descr = db_.GetBus(bus);
        if (!descr || descr->stops.empty()) {
            continue;
        }
        auto line = std::make_unique<svg::Polyline>();
        for (const auto& stop : descr->stops) {
            line->AddPoint(projector(db_.GetStop(stop)->place));
        }
        const auto& color = *color_it;
        line->SetStrokeColor(color);
        line->SetFillColor(svg::DefaultColor);
        line->SetStrokeWidth(settings_.line_width);
        line->SetStrokeLineCap(StrokeLineCap::ROUND);
        line->SetStrokeLineJoin(StrokeLineJoin::ROUND);
        doc.AddPtr(std::move(line));        
        if (++color_it == settings_.color_palette.end()) {
            color_it = settings_.color_palette.begin();
        }
    }
    //
    color_it = settings_.color_palette.begin();
    for (const auto& bus : buses) {
        const auto descr = db_.GetBus(bus);
        if (!descr || descr->stops.empty()) {
            continue;
        }
        const auto& color = *color_it;
        for (const auto& stop : descr->final_stops) {
             auto text = MakeBaseBusText(stop, bus, projector);
             text->SetFillColor(color);
             auto underlayer = MakeBaseBusText(stop, bus, projector);
             underlayer->SetFillColor(settings_.underlayer_color);
             underlayer->SetStrokeColor(settings_.underlayer_color);
             underlayer->SetStrokeWidth(settings_.underlayer_width);
             underlayer->SetStrokeLineCap(StrokeLineCap::ROUND);
             underlayer->SetStrokeLineJoin(StrokeLineJoin::ROUND);
             doc.AddPtr(std::move(underlayer));
             doc.AddPtr(std::move(text));
        }  
        if (++color_it == settings_.color_palette.end()) {
            color_it = settings_.color_palette.begin();
        }
    }
}
    
void MapRenderer::RenderStops(const SphereProjector& projector, svg::Document& doc) const {
    const auto& allstops = db_.GetStopIds();
    for (const auto& stop : allstops) {
        auto circle = std::make_unique<svg::Circle>();
        circle->SetCenter(projector(db_.GetStop(stop)->place));
        circle->SetRadius(settings_.stop_radius);
        circle->SetFillColor(svg::Color {"white"sv});
        doc.AddPtr(std::move(circle));
    }
    for (const auto& stop : allstops) {
        auto text = MakeBaseStopText(stop, projector);
        auto underlayer = MakeBaseStopText(stop, projector);
        text->SetFillColor(svg::Color {"black"sv});
        underlayer->SetFillColor(settings_.underlayer_color);
        underlayer->SetStrokeColor(settings_.underlayer_color);
        underlayer->SetStrokeWidth(settings_.underlayer_width);
        underlayer->SetStrokeLineCap(StrokeLineCap::ROUND);
        underlayer->SetStrokeLineJoin(StrokeLineJoin::ROUND);
        doc.AddPtr(std::move(underlayer));
        doc.AddPtr(std::move(text));
    }
}
    
std::unique_ptr<svg::Text> MapRenderer::MakeBaseBusText(const std::string_view& stop, const std::string_view& data, const SphereProjector& projector) const {
    auto text = std::make_unique<svg::Text>();
    text->SetPosition(projector(db_.GetStop(stop)->place));
    text->SetOffset(settings_.bus_label_offset);
    text->SetFontSize(settings_.bus_label_font_size);
    text->SetFontFamily("Verdana"s);
    text->SetFontWeight("bold"s);
    text->SetData(std::string{data});
    return text;
}
    
std::unique_ptr<svg::Text> MapRenderer::MakeBaseStopText(const std::string_view& stop,  const SphereProjector& projector) const {
    auto text = std::make_unique<svg::Text>();
    text->SetPosition(projector(db_.GetStop(stop)->place));
    text->SetOffset(settings_.stop_label_offset);
    text->SetFontSize(settings_.stop_label_font_size);
    text->SetFontFamily("Verdana"s);
    text->SetData(std::string{stop});
    return text;
}

}
