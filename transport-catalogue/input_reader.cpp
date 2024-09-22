#include "input_reader.h"

#include <algorithm>
#include <cassert>
#include <iterator>

using namespace geo;
using namespace transport;
    

static const std::string_view empty_string_view;

/**
 * Парсит строку вида "10.123,  -30.1837" и возвращает пару координат (широта, долгота)
 */
std::string_view ParseCoordinates(std::string_view str, Coordinates& result) {
    static const double nan = std::nan("");

    auto not_space = str.find_first_not_of(' ');
    auto comma = str.find(',');

    if (comma == str.npos) {
        result = {nan, nan};
        return str.substr(comma);
    }

    auto not_space2 = str.find_first_not_of(' ', comma + 1);
    auto comma2 = str.find(',', not_space2);

    double lat = std::stod(std::string(str.substr(not_space, comma - not_space)));
    double lng = std::stod(std::string(str.substr(not_space2, comma2 - not_space2)));

    result = {lat, lng};
    if (comma2 == str.npos) {
        return {};
    }
    return str.substr(comma2 + 1);
}

std::pair<std::string_view, int> ParseDistanceToStop(std::string_view str) {
    auto delim = str.find(" to ");
    return {str.substr(delim + 4), std::stoi(std::string(str.substr(0, delim - 1)))};
} 
 
/**
 * Парсит строку в пары типа <остановка, расстояние до нее>
 */
std::vector<transport::Dist2Stop> ParseDistancesToStops(std::string_view str) {
    std::vector<transport::Dist2Stop> result;
    while (!str.empty()) {
        auto not_space = str.find_first_not_of(' ');
        auto comma = str.find(',');
        result.push_back(ParseDistanceToStop(str.substr(not_space, comma - not_space)));
        str = comma != str.npos ? str.substr(comma + 1) : empty_string_view;
    }
    return result;
}

/**
 * Удаляет пробелы в начале и конце строки
 */
std::string_view Trim(std::string_view string) {
    const auto start = string.find_first_not_of(' ');
    if (start == string.npos) {
        return {};
    }
    return string.substr(start, string.find_last_not_of(' ') + 1 - start);
}

/**
 * Разбивает строку string на n строк, с помощью указанного символа-разделителя delim
 */
std::vector<std::string_view> Split(std::string_view string, char delim) {
    std::vector<std::string_view> result;

    size_t pos = 0;
    while ((pos = string.find_first_not_of(' ', pos)) < string.length()) {
        auto delim_pos = string.find(delim, pos);
        if (delim_pos == string.npos) {
            delim_pos = string.size();
        }
        if (auto substr = Trim(string.substr(pos, delim_pos - pos)); !substr.empty()) {
            result.push_back(substr);
        }
        pos = delim_pos + 1;
    }

    return result;
}

/**
 * Парсит маршрут.
 * Для кольцевого маршрута (A>B>C>A) возвращает массив названий остановок [A,B,C,A]
 * Для некольцевого маршрута (A-B-C-D) возвращает массив названий остановок [A,B,C,D,C,B,A]
 */
std::vector<std::string_view> ParseRoute(std::string_view route) {
    if (route.find('>') != route.npos) {
        return Split(route, '>');
    }

    auto stops = Split(route, '-');
    std::vector<std::string_view> results(stops.begin(), stops.end());
    results.insert(results.end(), std::next(stops.rbegin()), stops.rend());

    return results;
}

CommandDescription ParseCommandDescription(std::string_view line) {
    auto colon_pos = line.find(':');
    if (colon_pos == line.npos) {
        return {};
    }

    auto space_pos = line.find(' ');
    if (space_pos >= colon_pos) {
        return {};
    }

    auto not_space = line.find_first_not_of(' ', space_pos);
    if (not_space >= colon_pos) {
        return {};
    }

    return {std::string(line.substr(0, space_pos)),
            std::string(line.substr(not_space, colon_pos - not_space)),
            std::string(line.substr(colon_pos + 1))};
}

void InputReader::ParseLine(std::string_view line) {
    auto command_description = ParseCommandDescription(line);
    if (command_description) {
        commands_.push_back(std::move(command_description));
    }
}

void InputReader::ApplyCommands([[maybe_unused]] TransportCatalogue& catalogue) const {
    std::vector<const CommandDescription*> stop_cmd;
    std::vector<const CommandDescription*> bus_cmd;
    std::vector<std::pair<std::string_view, std::vector<transport::Dist2Stop>>> stop_dists;
    for (const auto& cmd : commands_) {
        if (cmd) {
            if (cmd.command == "Stop") {
                stop_cmd.push_back(&cmd);
            } else if (cmd.command == "Bus") {
                bus_cmd.push_back(&cmd);
            }
        }
    }
    for (const auto& cmd : stop_cmd) {
        Coordinates place;
        stop_dists.emplace_back(cmd->id, ParseDistancesToStops(ParseCoordinates(cmd->description, place)));      
        catalogue.AddStop(cmd->id, place);        
    }
    for (const auto& [id, dists] : stop_dists) {
        catalogue.AddDists(id, dists);
    }
    for (const auto& cmd : bus_cmd) {
        std::vector<std::string_view> route = ParseRoute(cmd->description);
        catalogue.AddBus(cmd->id, route);        
    }
}
