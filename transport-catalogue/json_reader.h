#pragma once

#include "domain.h"
#include "transport_catalogue.h"
#include "json.h"

class JsonReader {
public:    
    void ParseCommands(std::istream& in);
    
    const domain::RenderSettings& GetSettings() const;
    
    const domain::Commands& GetCommands() const;
private:    
    template <typename T>
    T Default() {
        return T();
    }

    template <typename T>
    T CastNode(const json::Node& node);

    template <typename T>
    T GetValueOrDefault(const json::Dict& node, std::string key) {
        for (auto ptr = node.find(key); ptr != node.end(); ptr = node.end()) {
            return CastNode<T>(ptr->second);
        }
        return Default<T>();
    }
    
    domain::Commands commands_;
    domain::RenderSettings settings_;
};
