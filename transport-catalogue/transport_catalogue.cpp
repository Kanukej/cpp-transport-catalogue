#include <unordered_set>

#include "transport_catalogue.h"
#include "geo.h"
#include <sstream>

using namespace transport;
using namespace geo;

static const std::set<std::string_view> empty_set;

std::string_view TransportCatalogue::AddId(const std::string_view id) {
    ids_.push_front(std::string {id});
    return {ids_.front().begin(), ids_.front().end()};
}

void TransportCatalogue::AddStop(const std::string_view id, const Coordinates place) {
    std::string_view stop_id = AddId(id);
    stops_.insert({stop_id, {stop_id, place}});
    busses4stop_.insert({stop_id, {}});
}

void TransportCatalogue::AddBus(const std::string_view id, const std::vector<std::string_view> stops) {
    std::string_view bus_id = AddId(id);
    std::vector<std::string_view> stops_ids;
    stops_ids.reserve(stops.size());
    for (const auto& stop : stops) {
        stops_ids.push_back(stops_.at(s).id);
        busses4stop_[stops_ids.back()].insert(bus_id);
    }
    busses_.insert({bus_id, {bus_id, std::move(stops_ids)}});
}

const BusDescription* TransportCatalogue::GetBus(const std::string_view id) const {
    auto bus_ptr = busses_.find(id);
    if (bus_ptr != busses_.end()) {
        return &bus_ptr->second;
    }
    return nullptr;
}

const std::optional<RouteStatistics> TransportCatalogue::GetStat(const BusDescription* bus) const {
    if (bus) {
        double route_length = 0.0;
        std::unordered_set<std::string_view> unique_stops;
        std::optional<StopDescription> prev_stop;
        for (const std::string_view& s : bus->stops) {
            const auto stop = stops_.at(s);
            unique_stops.insert(s);
            if (prev_stop) {
                route_length += ComputeDistance(prev_stop->place, stop.place);
            }
            prev_stop = stop;
        }
        return RouteStatistics {route_length, bus->stops.size(), unique_stops.size()};
    }
    return std::nullopt;
}

const std::set<std::string_view>& TransportCatalogue::GetBusses4Stop(const std::string_view id) const {
    auto busses4stop_ptr = busses4stop_.find(id);
    if (busses4stop_ptr != busses4stop_.end()) {
        return busses4stop_ptr->second;
    } else {
        std::stringstream msg;
        msg << "Stop " << id << ": not found";
        throw std::out_of_range(msg.str());
    }
    return empty_set;
}
