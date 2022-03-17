#pragma once

#include "nlohmann/json.hpp"
#include <unordered_map>
#include <vector>

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

    typedef std::vector<ClientState> ServerStateReport;

    inline void to_json(nlohmann::json& j, const ServerStateReport& serverStateReport) {
        j = nlohmann::json::array();

        for (const auto& client : serverStateReport) {
            j.push_back(client);
        }
    }

    inline void from_json(const nlohmann::json& j, ServerStateReport& serverStateReport) {
        for (auto& client : j) {
            serverStateReport.push_back(client);
        }
    }
}
