#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"
#include "json.h"

namespace handler {
    
class CatalogueConstructor {
public:
    CatalogueConstructor(transport::TransportCatalogue& db, const domain::RoutingSettings& settings);
    void FillFromCommands(const domain::Commands& commands);
private:
    transport::TransportCatalogue& db_;
    const domain::RoutingSettings& settings_;
};

class RequestHandler {
public:
    // MapRenderer понадобится в следующей части итогового проекта
    RequestHandler(const transport::TransportCatalogue& db, const renderer::MapRenderer& renderer, const router::TransportRouter& router);

    // Возвращает информацию о маршруте (запрос Bus)
    std::optional<transport::RouteStatistics> GetBusStat(const std::string_view& bus_name) const;

    // Возвращает маршруты, проходящие через
    const std::set<transport::BusPtr>* GetBusesByStop(const std::string_view& stop_name) const;

    // Этот метод будет нужен в следующей части итогового проекта
    const svg::Document RenderMap() const;
    
    json::Document ApplyCommands(const domain::Commands& commands) const;

private:
    // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
    const transport::TransportCatalogue& db_;
    const renderer::MapRenderer& renderer_;
    const router::TransportRouter& router_;
};
    
}
