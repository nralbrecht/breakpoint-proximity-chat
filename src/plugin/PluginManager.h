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
#include "PluginConfig.h"
#include "NetworkManager.h"
#include "ClientStateManager.h"


class PluginManager
{
private:
    std::unique_ptr<GameHandler> gameHandler;
    std::unique_ptr<NetworkManager> networkManager;
    std::unique_ptr<ClientStateManager> clientStateManager;

    std::unique_ptr<asio2::timer> timer;
    static const unsigned int stateUpdateTimerId = 0;
    static const unsigned int gameHandlerReconnectTimerId = 1;

    struct TS3Functions ts3Functions;

    GRB_state_t ownState;
    bool isUsingRadio = false;
public:
    static const std::string NAME;
    static const std::string VERSION;
    static const int API_VERSION;
    static const std::string AUTHOR;
    static const std::string DESCRIPTION;

    PluginManager(struct TS3Functions ts3Functions);
    ~PluginManager();

    void radioActivate(bool active);
    void updateOwnState();
    void updatePositions();

    void onHotkeyEvent(const char* keyword);
    void currentServerConnectionChanged(uint64 serverConnectionHandlerID);
    void onConnectStatusChangeEvent(uint64 serverConnectionHandlerID, int newStatus, unsigned int errorNumber);
    void onClientMoveEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, const char* moveMessage);
    void onClientMoveMovedEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, anyID moverID, const char* moverName, const char* moverUniqueIdentifier, const char* moveMessage);
    void onCustom3dRolloffCalculationClientEvent(uint64 serverConnectionHandlerID, anyID clientID, float distance, float* volume);
};
