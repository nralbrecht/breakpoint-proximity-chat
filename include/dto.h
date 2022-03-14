#pragma once

#include "nlohmann/json.hpp"
#include <unordered_map>

namespace DTO {
    struct ClientState {
        std::string uuid;
        float x;
        float y;
        float z;
        bool isUsingRadio;
    };

    inline void to_json(nlohmann::json& j, const ClientState& clientState) {
        j = nlohmann::json{ {"uuid", clientState.uuid}, {"x", clientState.x}, {"y", clientState.y}, {"z", clientState.z}, {"isUsingRadio", clientState.isUsingRadio} };
    }

    inline void from_json(const nlohmann::json& j, ClientState& clientState) {
        j.at("uuid").get_to(clientState.uuid);
        j.at("x").get_to(clientState.x);
        j.at("y").get_to(clientState.y);
        j.at("z").get_to(clientState.z);
        j.at("isUsingRadio").get_to(clientState.isUsingRadio);
    }

    struct ServerStateReport {
        std::unordered_map<std::string, ClientState> clients;
    };

    inline void to_json(nlohmann::json& j, const ServerStateReport& serverStateReport) {
        j = nlohmann::json::object();

        for (const auto& [_, client] : serverStateReport.clients) {
            j[client.uuid] = client;
        }
    }

    inline void from_json(const nlohmann::json& j, ServerStateReport& serverStateReport) {
        for (auto& [uuid, client] : j.items()) {
            serverStateReport.clients[uuid] = client;
        }
    }
}
