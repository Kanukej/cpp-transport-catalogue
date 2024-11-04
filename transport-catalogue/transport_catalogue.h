#pragma once

#include <vector>
#include <unordered_set>
#include <set>
#include <stdexcept>
#include <string>
#include <optional>
#include <unordered_map>
#include <iostream>
#include <forward_list>

#include "geo.h"
#include "domain.h"

namespace transport {
    
    using StopsMap = std::unordered_map<std::string_view, int>;
    using BusPtr = std::string_view;

    
    struct StopDescription {
        std::string_view id;
        geo::Coordinates place;
        StopsMap distances;
    };

    struct BusDescription {
        std::string_view id;
        std::vector<std::string_view> stops;
        std::vector<std::string_view> final_stops;
    };

    struct RouteStatistics {
        int dist = 0;
        size_t stops_count = 0;
        size_t unique_stops = 0;
        double curvature = 0.0;
    };
    
    using StopContainer = std::unordered_map<std::string_view, StopDescription>;
    using BusContainer = std::unordered_map<std::string_view, BusDescription>;
    
    class TransportStopIterator {
    public:
        TransportStopIterator() = default;
        TransportStopIterator(const StopContainer::const_iterator& it) : it_(it) {
            //
        }
    
        const geo::Coordinates& operator*() const {
            return it_->second.place;
        }
        const geo::Coordinates* operator->() const {
            return &it_->second.place;
        }
        TransportStopIterator& operator++() {
            ++it_;
            return *this;
        }
        bool operator==(const TransportStopIterator& other) {
            return it_ == other.it_;
        }
        bool operator!=(const TransportStopIterator& other) {
            return it_ != other.it_;
        }
    private:
        StopContainer::const_iterator it_;
    };
    
    class TransportCatalogue {
    public:
        void AddStop(const std::string_view id, const geo::Coordinates place);
        void AddBus(const std::string_view id, const std::vector<std::string_view>& stops, const std::vector<std::string_view>& final_stops);
        void AddDistance(const std::string_view from, const std::string_view to, const int dists);
        const BusDescription* GetBus(const std::string_view id) const;
        const std::optional<RouteStatistics> GetStat(const BusDescription* bus) const;
        const std::set<BusPtr>* GetBusses4Stop(const std::string_view id) const;
        std::vector<geo::Coordinates> GetStops() const;
        std::set<std::string_view> GetStopIds() const;
        std::vector<std::string_view> GetBuses() const;
        const StopDescription* GetStop(const std::string_view id) const;
    private:
        std::string_view AddId(const std::string_view id);
    private:
        std::forward_list<std::string> ids_;
        StopContainer stops_;
        BusContainer buses_;
        std::unordered_map<std::string_view, std::set<std::string_view>> busses4stop_;
    };
    
}
