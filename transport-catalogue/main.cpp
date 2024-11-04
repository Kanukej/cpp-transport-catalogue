#include "transport_catalogue.h"
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

int main() {
    TransportCatalogue db;
    JsonReader reader;
    reader.ParseCommands(cin);

    const auto& commands = reader.GetCommands();
    const auto& settings = reader.GetSettings();

    CatalogueConstructor constructor(db);
    MapRenderer renderer(settings, db);
    constructor.FillFromCommands(commands);
    RequestHandler applyer(db, renderer);
    const auto& ans = applyer.ApplyCommands(commands);

    Print(ans, cout);    
}
