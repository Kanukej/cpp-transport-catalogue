#pragma once

#include "geo.h"
#include "svg.h"

#include <string>
#include <vector>
#include <unordered_map>
#include <forward_list>
#include <variant>

namespace domain {

enum class StatType {
    Bus,
    Stop,
    Map
};

struct Dist2Stop {
    std::string_view stop;
    int distance;
};

struct StopDistance {
    std::string stop;
    int distance;
};

struct StopRequest {
    std::string name;
    geo::Coordinates place;
    std::vector<StopDistance> road_distances;
};

struct BusRequest {
    std::string name;
    std::vector<std::string_view> stops;
    std::vector<std::string_view> final_stops;
    bool is_roundtrip;
};

struct StatRequest {
    int id;
    StatType type;
    std::string name;
};

struct Commands {
    std::vector<StopRequest> stop_requests;
    std::vector<BusRequest> bus_requests;
    std::vector<StatRequest> stat_requests;
    std::string_view AddId(const std::string& id);
private:
    std::forward_list<std::string> ids_;
};

struct Offset {
    double dx;
    double dy;
};

using ColorPalette = std::vector<svg::Color>;

struct RenderSettings {
    double width;
    double height;
    double padding;
    double line_width;
    double stop_radius;
    double bus_label_font_size;
    svg::Point bus_label_offset;
    int stop_label_font_size;
    svg::Point stop_label_offset;
    svg::Color underlayer_color;
    double underlayer_width;
    ColorPalette color_palette;    
};

}
