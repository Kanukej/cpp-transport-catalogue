#include "transport_catalogue.h"
#include "transport_router.h"
#include "json.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "request_handler.h"

#include <iostream>

using namespace std;
using namespace transport;
using namespace json;
using namespace renderer;
using namespace handler;
using namespace router;

int main() {
    TransportCatalogue db;
    JsonReader reader;
    reader.ParseCommands(cin);

    const auto& commands = reader.GetCommands();
    const auto& settings = reader.GetSettings();
    const auto& base_settings = reader.GetBaseSettings();

    CatalogueConstructor constructor(db, base_settings);
    MapRenderer renderer(settings, db);
    constructor.FillFromCommands(commands);
    TransportRouter router(db);
    RequestHandler applyer(db, renderer, router);
    const auto& ans = applyer.ApplyCommands(commands);

    Print(ans, cout);    
}
