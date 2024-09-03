#include <string>
#include <iostream>
#include <iomanip>
#include <stdexcept>

#include "stat_reader.h"
#include "input_reader.h"

using namespace std;
using namespace geo;
using namespace transport;

void ParseAndPrintStat(const TransportCatalogue& transport_catalogue, std::string_view request,
                       std::ostream& output) {
    auto r = Trim(request);
    if (r.substr(0, 3) == "Bus") {
        const std::string_view bus_id = Trim(r.substr(3));
        const auto bus = transport_catalogue.GetBus(bus_id);
        const auto stat = transport_catalogue.GetStat(bus);
        output << "Bus "s << bus_id << ": "s;
        if (!stat) {
            output << "not found"s << std::endl;
            return;
        }        
        output << stat->stops_count << " stops on route, "s << stat->unique_stops << " unique stops, "s << std::setprecision(6) << stat->length << " route length"s << std::endl;
    } else if (r.substr(0, 4) == "Stop") {
        const std::string_view stop_id = Trim(r.substr(4));
        try {
            const auto busses4stop = transport_catalogue.GetBusses4Stop(stop_id);
            output << "Stop " << stop_id;
            if (busses4stop.empty()) {
                output << ": no buses";
            } else {
                output << ": buses";
                for (const auto& b : busses4stop) {
                    output << " " << b;
                }
            }
            output << std::endl;
        } catch (std::out_of_range& ex) {
            output << ex.what() << std::endl;
        }
    }
}
