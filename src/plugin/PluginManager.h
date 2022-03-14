#pragma once

#include <string>
#include <memory>
#include <unordered_map>

#include "teamspeak/public_definitions.h"
#include "teamspeak/public_errors.h"
#include "asio2/base/timer.hpp"
#include "ts3_functions.h"

#include "dto.h"
#include "Logger.h"
#include "GameHandler.h"
#include "NetworkManager.h"


class PluginManager
{
private:
    std::unique_ptr<GameHandler> gameHandler;
    std::unique_ptr<NetworkManager> networkManager;

    std::unique_ptr<asio2::timer> timer;
    static const unsigned int stateUpdateTimerId = 0;
    static const unsigned int gameHandlerReconnectTimerId = 1;

    struct TS3Functions ts3Functions;
    const std::unordered_map<std::string, anyID> &uuidForAvailableClients;

    GRB_state_t ownState;
    bool isUsingRadio = false;
public:
    PluginManager(struct TS3Functions ts3Functions, const std::unordered_map<std::string, anyID> &uuidForAvailableClients);
    ~PluginManager();

    void radioActivate(bool active);
    void updateOwnState();
    void updatePositions(DTO::ServerStateReport serverStateReport);
};
