#include "request_handler.h"
#include "domain.h"
#include "json_builder.h"

#include <sstream>

using namespace transport;
using namespace handler;
using namespace json;
using namespace domain;


RequestHandler::RequestHandler(const TransportCatalogue& db, const renderer::MapRenderer& renderer, const router::TransportRouter& router) : db_(db), renderer_(renderer), router_(router) {
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
    json::Builder ans;
    ans.StartArray();
    for (const auto& cmd : commands.stat_requests) {
        ans.StartDict();
        switch (cmd.type) {
            case StatType::Bus: {
                const auto bus = db_.GetBus(cmd.name);
                const auto stat = db_.GetStat(bus);
                ans.Key("request_id").Value(cmd.id);
                if (!stat) {
                    ans.Key("error_message").Value("not found");
                } else {
                    ans.Key("curvature").Value(stat->curvature)
                       .Key("route_length").Value(stat->dist)
                       .Key("stop_count").Value(static_cast<int>(stat->stops_count))
                       .Key("unique_stop_count").Value(static_cast<int>(stat->unique_stops));
                }
                break;
            }
            case StatType::Stop: {
                const auto buses4stop = db_.GetBusses4Stop(cmd.name);
                ans.Key("request_id").Value(cmd.id);
                if (!buses4stop) {
                    ans.Key("error_message").Value("not found");
                } else {
                    ans.Key("buses").StartArray();
                    for (const auto& bus : *buses4stop) {
                        ans.Value(std::string(bus));
                    }
                    ans.EndArray();
                }
                break;
            }
            case StatType::Map: {
                const auto& doc = RenderMap();
                std::stringstream map;
                doc.Render(map);
                ans.Key("request_id").Value(cmd.id)
                   .Key("map").Value(map.str());
                break;
            }
            case StatType::Route: {
                auto path = router_.GetPath(cmd.from, cmd.to);
                if (path) {
                    ans.Key("request_id").Value(cmd.id)
                       .Key("total_time").Value(path->time)
                       .Key("items").StartArray();
                    for (const auto& d : path->route) {
                        if (d.type == transport::PathType::Wait) {
                            ans.StartDict()
                               .Key("type").Value("Wait")
                               .Key("stop_name").Value(std::string(d.id))
                               .Key("time").Value(d.time)
                               .EndDict();
                        } else if (d.type == transport::PathType::Bus) {
                            ans.StartDict()
                               .Key("type").Value("Bus")
                               .Key("bus").Value(std::string(d.id))
                               .Key("span_count").Value(d.span.value())
                               .Key("time").Value(d.time)
                               .EndDict();
                        }
                    }
                    ans.EndArray();
                } else {
                    ans.Key("request_id").Value(cmd.id)
                       .Key("error_message").Value("not found");
                }
                break;
            }
        }
        ans.EndDict();
    }
    return Document(ans.EndArray().Build());
}

CatalogueConstructor::CatalogueConstructor(transport::TransportCatalogue& db, const domain::RoutingSettings& settings) : db_(db), settings_(settings) {
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
    db_.BuildMapGraph(settings_.bus_velocity, settings_.bus_wait_time);
}
