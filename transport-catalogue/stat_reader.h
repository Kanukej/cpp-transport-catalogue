#pragma once

#include <iosfwd>
#include <string_view>

#include "transport_catalogue.h"

void ParseAndPrintStat(const transport::TransportCatalogue& tansport_catalogue, std::string_view request,
                       std::ostream& output);

void ReadAndApply(transport::TransportCatalogue& catalogue, std::istream& input);

void ReadAndEval(const transport::TransportCatalogue& catalogue, std::istream& input, std::ostream& output);
