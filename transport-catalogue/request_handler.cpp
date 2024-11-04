#include "request_handler.h"
#include "domain.h"

#include <sstream>

using namespace transport;
using namespace handler;
using namespace json;
using namespace domain;


RequestHandler::RequestHandler(const TransportCatalogue& db, const renderer::MapRenderer& renderer) : db_(db), renderer_(renderer) {
    //
}

std::optional<RouteStatistics> RequestHandler::GetBusStat(const std::string_view& bus_name) const {
    const auto bus = db_.GetBus(bus_name);
    return db_.GetStat(bus);    
}

const std::set<BusPtr>* RequestHandler::GetBusesByStop(const std::string_view& stop_name) const {
    return db_.GetBusses4Stop(stop_name);    
}

const svg::Document RequestHandler::RenderMap() const {
    return renderer_.Render();
}


json::Document RequestHandler::ApplyCommands(const domain::Commands& commands) const {
    Array ans;
    for (const auto& cmd : commands.stat_requests) {
        json::Dict result;
        switch (cmd.type) {
            case StatType::Bus: {
                const auto bus = db_.GetBus(cmd.name);
                const auto stat = db_.GetStat(bus);
                result["request_id"] = cmd.id;
                if (!stat) {
                    result["error_message"] = "not found";
                } else {
                    result["curvature"] = stat->curvature;
                    result["route_length"] = stat->dist;
                    result["stop_count"] = static_cast<int>(stat->stops_count);
                    result["unique_stop_count"] = static_cast<int>(stat->unique_stops);
                }
                break;
            }
            case StatType::Stop: {
                const auto buses4stop = db_.GetBusses4Stop(cmd.name);
                result["request_id"] = cmd.id;
                if (!buses4stop) {
                    result["error_message"] = "not found";
                } else {
                    Array buses;
                    for (const auto& bus : *buses4stop) {
                        buses.push_back(std::string(bus));
                    }
                    result["buses"] = buses;
                }
                break;
            }
            case StatType::Map:
                result["request_id"] = cmd.id;
                const auto& doc = RenderMap();
                std::stringstream map;
                doc.Render(map);
                result["map"] = map.str();
                break;
        }
        ans.push_back(result);
    }
    return Document(ans);
}

CatalogueConstructor::CatalogueConstructor(transport::TransportCatalogue& db) : db_(db) {
    //
}

void CatalogueConstructor::FillFromCommands(const domain::Commands& commands) {
    for (const auto& cmd : commands.stop_requests) {
        db_.AddStop(cmd.name, cmd.place);        
    }
    for (const auto& cmd : commands.stop_requests) {
        for (const auto& to : cmd.road_distances) {
            db_.AddDistance(cmd.name, to.stop, to.distance);
        }       
    }
    for (const auto& cmd : commands.bus_requests) {
        db_.AddBus(cmd.name, cmd.stops, cmd.final_stops);        
    }
}
