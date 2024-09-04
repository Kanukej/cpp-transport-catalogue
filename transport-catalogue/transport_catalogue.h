#pragma once

#include <vector>
#include <set>
#include <stdexcept>
#include <string>
#include <optional>
#include <unordered_map>
#include <iostream>
#include <forward_list>

#include "geo.h"

namespace transport {

    struct StopDescription {
        std::string_view id;
        geo::Coordinates place;
    };

    struct BusDescription {
        std::string_view id;
        std::vector<std::string_view> stops;
    };

    struct RouteStatistics {
        double length = 0.0;
        size_t stops_count = 0;
        size_t unique_stops = 0;
    };

    class TransportCatalogue {
    public:
        void AddStop(const std::string_view id, const geo::Coordinates place);
        void AddBus(const std::string_view id, std::vector<std::string_view> stops);
        const BusDescription* GetBus(const std::string_view id) const;
        const std::optional<RouteStatistics> GetStat(const BusDescription* bus) const;
        const std::set<std::string_view>& GetBusses4Stop(const std::string_view id) const;
    private:
        std::string_view AddId(const std::string_view id);
    private:
        std::forward_list<std::string> ids_;
        std::unordered_map<std::string_view, StopDescription> stops_;
        std::unordered_map<std::string_view, BusDescription> busses_;
        std::unordered_map<std::string_view, std::set<std::string_view>> busses4stop_;
    };
    
}
