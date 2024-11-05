#include "json_reader.h"
#include "json.h"
#include "map_renderer.h"

#include <algorithm>
#include <sstream>

using namespace json;
using namespace domain;
using namespace std::literals;

void JsonReader::ParseBaseRequest(const json::Dict& root) {
    for (auto ptr = root.find("base_requests"); ptr != root.end(); ptr = root.end()) {
        const auto& requests = ptr->second.AsArray();
        for (const auto& r : requests) {
            const auto& base_request = r.AsMap();
            std::string type = base_request.at("type").AsString();
            std::string name = base_request.at("name").AsString();
            if (type == "Stop") {
                StopRequest ans;
                ans.name = name;
                ans.place = {base_request.at("latitude").AsDouble(), base_request.at("longitude").AsDouble()};
                if (base_request.count("road_distances")) {
                    for (const auto& [id, dist] : base_request.at("road_distances").AsMap()) {
                        ans.road_distances.push_back({id, dist.AsInt()});
                    }
                }
                commands_.stop_requests.push_back(ans);
            } else if (type == "Bus") {
                BusRequest ans;
                ans.name = name;
                const auto& stops = base_request.at("stops").AsArray();          
                for (const auto& stop : stops) {
                    ans.stops.push_back(commands_.AddId(stop.AsString()));
                }
                ans.is_roundtrip = base_request.at("is_roundtrip").AsBool();
                ans.final_stops.push_back(ans.stops.front());
                if (!ans.is_roundtrip) {
                    if (ans.stops.front() != ans.stops.back()) {
                        ans.final_stops.push_back(ans.stops.back());
                    }
                    std::vector<std::string_view> return_stops;
                    for (auto it = ans.stops.rbegin(); it != ans.stops.rend();++it) {
                        if (it == ans.stops.rbegin()) {
                            continue;
                        }
                        return_stops.push_back(*it);
                    }
                    for (const auto& s : return_stops) {
                        ans.stops.push_back(s);
                    }
                }
                commands_.bus_requests.push_back(ans);
            }
        }
    } 
}

void JsonReader::ParseStatRequest(const json::Dict& root) {
    for (auto ptr = root.find("stat_requests"); ptr != root.end(); ptr = root.end()) {
        const auto& requests = ptr->second.AsArray();
        for (const auto& req : requests) {
            StatRequest ans;
            const auto& r = req.AsMap();
            ans.id = GetValueOrDefault<int>(r, "id");
            std::string type = GetValueOrDefault<std::string>(r, "type");
            if (type == "Bus") {
                ans.type = StatType::Bus;
                ans.name = GetValueOrDefault<std::string>(r, "name");
            } else if (type == "Stop") {
                ans.type = StatType::Stop;
                ans.name = GetValueOrDefault<std::string>(r, "name");
            } else if (type == "Map") {
                ans.type = StatType::Map;
            }
            commands_.stat_requests.push_back(ans);
        }
    }
}

void JsonReader::ParseSettings(const json::Dict& root) {    
    for (auto ptr = root.find("render_settings"); ptr != root.end(); ptr = root.end()) {
        const auto& s = ptr->second.AsMap();
        settings_.width = std::move(GetValueOrDefault<double>(s, "width"));
        settings_.height = std::move(GetValueOrDefault<double>(s, "height"));
        settings_.padding = std::move(GetValueOrDefault<double>(s, "padding"));
        settings_.line_width = std::move(GetValueOrDefault<double>(s, "line_width"));
        settings_.stop_radius = std::move(GetValueOrDefault<double>(s, "stop_radius"));
        settings_.bus_label_font_size = std::move(GetValueOrDefault<double>(s, "bus_label_font_size"));
        settings_.bus_label_offset = std::move(GetValueOrDefault<svg::Point>(s, "bus_label_offset"));
        settings_.stop_label_font_size = std::move(GetValueOrDefault<int>(s, "stop_label_font_size"));
        settings_.stop_label_offset = std::move(GetValueOrDefault<svg::Point>(s, "stop_label_offset"));
        settings_.underlayer_color = std::move(GetValueOrDefault<svg::Color>(s, "underlayer_color"));
        settings_.underlayer_width = std::move(GetValueOrDefault<double>(s, "underlayer_width"));
        settings_.color_palette = std::move(GetValueOrDefault<ColorPalette>(s, "color_palette"));
    }
}

void JsonReader::ParseCommands(std::istream& in) {
    Document doc = json::Load(in);
    const auto& root = doc.GetRoot().AsMap();
    ParseBaseRequest(root);
    ParseStatRequest(root);
    ParseSettings(root);    
}

const RenderSettings& JsonReader::GetSettings() const {
    return settings_;
}

const Commands& JsonReader::GetCommands() const {
    return commands_;
}

template <>
svg::Color JsonReader::Default() {
    return svg::DefaultColor;
}

template <>
svg::Point JsonReader::Default() {
    return svg::Point{0.,0.};
}


template <>
int JsonReader::CastNode(const json::Node& node) {
    return node.AsInt();
}

template <>
double JsonReader::CastNode(const json::Node& node) {
    return node.AsDouble();
}

template <>
json::Array JsonReader::CastNode(const json::Node& node) {
    return node.AsArray();
}

template <>
json::Dict JsonReader::CastNode(const json::Node& node) {
    return node.AsMap();
}

template <>
svg::Point JsonReader::CastNode(const json::Node& node) {
    if (node.IsArray()) {
        const auto& components = node.AsArray();
        if (components.size() == 2) {
            return svg::Point{CastNode<double>(components[0]), CastNode<double>(components[1])};
        }
    }
    return Default<svg::Point>();
}

template <>
svg::Color JsonReader::CastNode(const json::Node& node) {
    if (node.IsString()) {
        return svg::Color(node.AsString());
    } else if (node.IsArray()) {
        const auto& components = node.AsArray();
        std::stringstream ss;        
        if (components.size() == 3) {
            ss << "rgb("sv << CastNode<int>(components[0]) << ","sv << CastNode<int>(components[1]) << ","sv << CastNode<int>(components[2]);
        }
        if (components.size() == 4) {
            ss << "rgba("sv << CastNode<int>(components[0]) << ","sv << CastNode<int>(components[1]) << ","sv << CastNode<int>(components[2]) << ","sv << CastNode<double>(components[3]);
        }
        ss << ")"sv;
        return svg::Color(ss.str());
    }
    return Default<svg::Color>();
}

template <>
ColorPalette JsonReader::CastNode(const json::Node& node) {
    if (node.IsArray()) {
        ColorPalette result;
        for (const auto& c : node.AsArray()) {
            result.push_back(CastNode<svg::Color>(c));
        }
        return result;
    }
    return Default<ColorPalette>();
}
