#include <unordered_set>

#include "transport_catalogue.h"
#include "geo.h"
#include <sstream>

using namespace transport;
using namespace geo;

std::string_view TransportCatalogue::AddId(const std::string_view id) {
    ids_.push_front(std::string {id});
    return {ids_.front().begin(), ids_.front().end()};
}

void TransportCatalogue::AddStop(const std::string_view id, const Coordinates place) {
    std::string_view stop_id = AddId(id);
    stops_.insert({stop_id, {stop_id, place, {}}});
    busses4stop_.insert({stop_id, {}});
}

void TransportCatalogue::AddBus(const std::string_view id, const std::vector<std::string_view>& stops, const std::vector<std::string_view>& final_stops) {
    std::string_view bus_id = AddId(id);
    std::vector<std::string_view> stops_ids;
    stops_ids.reserve(stops.size());
    for (const auto& stop : stops) {
        stops_ids.push_back(stops_.at(stop).id);
        busses4stop_[stops_ids.back()].insert(bus_id);
    }
    std::vector<std::string_view> final_stops_ids;
    for (const auto& stop : final_stops) {
        final_stops_ids.push_back(stops_.at(stop).id);
    }
    buses_.insert({bus_id, {bus_id, std::move(stops_ids), std::move(final_stops_ids)}});
}

void TransportCatalogue::AddDistance(const std::string_view from, const std::string_view to, const int dist) {
    stops_.at(from).distances[stops_.at(to).id] = dist;
}

void TransportCatalogue::BuildMapGraph(const int velocity, const TimeUnit time) {
    std::vector<const StopDescription*> descr;
    for (const auto& [name, sd] : stops_) {
        const auto idx = std::pair<graph::VertexId, graph::VertexId> {stop_map_.AddVertex(), stop_map_.AddVertex()} ;
        map2stop_[name] = idx;
        stop2map_[idx.first] = name;
        descr.push_back(&sd);
        stop_map_.AddEdge({idx.first, idx.second, time});
    }
    for (const auto& [id, bus] : buses_) {
        if (bus.stops.empty()) {
            continue;
        }
        std::vector<const DoubleStop*> graph_stops;
        graph_stops.reserve(bus.stops.size());
        for (auto from_it = bus.stops.begin(); from_it != bus.stops.end(); ++from_it) {
            const auto& from = map2stop_.at(*from_it);
            graph_stops.push_back(&from);
        }
        for (size_t from = 0; from < graph_stops.size() - 1; ++from) {
            double travel_time = 0.;
            for (size_t to = from + 1; to < graph_stops.size(); ++to) {
                const auto dist = GetDistance(*descr.at(graph_stops.at(to - 1)->first / 2), *descr.at(graph_stops.at(to)->first / 2));
                if (dist) {
                    travel_time += (60 * static_cast<double>(dist.value()) / velocity) / 1000;
                    bus2map_[stop_map_.AddEdge({graph_stops.at(from)->second, graph_stops.at(to)->first, travel_time})] = {id, to - from};
                }
            }
        }
    }
}

std::optional<RouteDescription> TransportCatalogue::DescribePath(const graph::EdgeId& id) const {
    const auto& edge = stop_map_.GetEdge(id);
    if (edge.weight > TimeUnit {0}) {
        const auto stop_it = stop2map_.find(edge.from);
        if (stop_it != stop2map_.end()) {
            RouteDescription ans {PathType::Wait, edge.weight, stop_it->second, std::nullopt};
            return ans;
        } else {
            const auto span_bus = bus2map_.at(id);
            RouteDescription ans {PathType::Bus, edge.weight, span_bus.first, span_bus.second};
            return ans;
        }
    }
    return std::nullopt;
}

graph::VertexId TransportCatalogue::GetStopGraphId(const std::string_view& id) const {
    return map2stop_.at(id).first;
}

const graph::DirectedWeightedGraph<TimeUnit>& TransportCatalogue::GetMapGraph() const {
    return stop_map_;
}

const BusDescription* TransportCatalogue::GetBus(const std::string_view id) const {
    auto bus_ptr = buses_.find(id);
    if (bus_ptr != buses_.end()) {
        return &bus_ptr->second;
    }
    return nullptr;
}

std::optional<int> TransportCatalogue::GetDistance(const StopDescription& from, const StopDescription& to) const {
    const auto& distances = from.distances;
    auto dist_ptr = distances.find(to.id);
    if (dist_ptr != distances.end()) {
        return dist_ptr->second;
    } else if (to.distances.end() != (dist_ptr = to.distances.find(from.id))) {
        return dist_ptr->second;
    }
    return std::nullopt;
}

const std::optional<RouteStatistics> TransportCatalogue::GetStat(const BusDescription* bus) const {
    if (bus) {
        double route_length = 0.0;
        int route_dist = 0;
        std::unordered_set<std::string_view> unique_stops;
        std::optional<StopDescription> prev_stop;
        for (const std::string_view& s : bus->stops) {
            const auto stop = stops_.at(s);
            unique_stops.insert(s);
            if (prev_stop) {
                const auto dist = GetDistance(*prev_stop, stop);
                if (dist) {
                    route_dist += dist.value();
                } else {
                    std::stringstream ss;
                    ss << "distance between stop " << s << " and stop " << prev_stop->id << " not found in base";
                    throw std::out_of_range(ss.str());
                }
                route_length += ComputeDistance(prev_stop->place, stop.place);
            }
            prev_stop = stop;
        }
        return RouteStatistics {route_dist, bus->stops.size(), unique_stops.size(), route_dist / route_length};
    }
    return std::nullopt;
}

const std::set<BusPtr>* TransportCatalogue::GetBusses4Stop(const std::string_view id) const {
    auto busses4stop_ptr = busses4stop_.find(id);
    if (busses4stop_ptr != busses4stop_.end()) {
        return &busses4stop_ptr->second;
    }
    return nullptr;
}

std::vector<geo::Coordinates> TransportCatalogue::GetStops() const {
    std::set<std::string_view> stops = GetStopIds();
    std::vector<geo::Coordinates> ans;
    ans.reserve(stops.size());
    for (const auto& [id, descr] : stops_) {
        if (stops.count(id) > 0) {
            ans.push_back(descr.place);
        }
    }
    return ans;
}

std::set<std::string_view> TransportCatalogue::GetStopIds() const {
    std::set<std::string_view> stops;
    for (const auto& [id, descr] : buses_) {
        for (const auto& s : descr.stops) {
            stops.insert(s);
        }
    }
    return stops;
}

std::vector<std::string_view> TransportCatalogue::GetBuses() const {
    std::vector<std::string_view> buses;
    for (const auto& [id, descr] : buses_) {
        buses.push_back(id);
    }
    return buses;
}

const StopDescription* TransportCatalogue::GetStop(const std::string_view id) const {
    const auto stop_ptr = stops_.find(id);
    if (stop_ptr != stops_.end()) {
        return &stop_ptr->second;
    }
    return nullptr;
}
