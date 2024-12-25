#pragma once

#include "transport_catalogue.h"
#include "graph.h"
#include "router.h"

namespace router {
    
class TransportRouter {
public:
    explicit TransportRouter(const transport::TransportCatalogue& db);
    
    std::optional<transport::PathDescription> GetPath(const std::string_view& from, const std::string_view& to) const;
        
private:
    const transport::TransportCatalogue& db_;
    const graph::Router<transport::TimeUnit> router_;
};
}
