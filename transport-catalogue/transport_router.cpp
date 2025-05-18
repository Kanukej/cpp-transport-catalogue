#include "transport_router.h"

namespace router {
    
    TransportRouter::TransportRouter(const transport::TransportCatalogue& db) : db_(db), router_(graph::Router (db_.GetMapGraph())) {
    }
    
    std::optional<transport::PathDescription> TransportRouter::GetPath(const std::string_view& from, const std::string_view& to)  const {
        auto from_id = db_.GetStopGraphId(from);
        auto to_id = db_.GetStopGraphId(to);
        auto path = router_.BuildRoute(from_id, to_id);
        if (path) {
            std::vector<transport::RouteDescription> result;
            for (const auto& id : path->edges) {
                auto descr = db_.DescribePath(id);
                if (descr) {
                    const transport::PathType type = descr->type;
                    switch (type) {
                        case transport::PathType::Wait:
                            result.push_back(std::move(*descr));
                            break;
                        case transport::PathType::Bus:
                            result.push_back(std::move(*descr));
                            break;
                        default:
                            break;
                    }
                }
            }
            transport::PathDescription ans {path->weight, std::move(result)};
            return ans;
        }
        return std::nullopt;
    }
    
}
