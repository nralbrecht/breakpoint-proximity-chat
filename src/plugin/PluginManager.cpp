#include "PluginManager.h"


PluginManager::PluginManager(struct TS3Functions ts3Functions) {
    this->ts3Functions = ts3Functions;

    try
    {
        gameHandler = std::make_unique<GameHandler>();

        ts3Functions.logMessage("PluginManager initialize", LogLevel_INFO, "GRB", ts3Functions.getCurrentServerConnectionHandlerID());
    }
    catch(const std::exception& e)
    {
        ts3Functions.logMessage(e.what(), LogLevel_ERROR, "GRB", ts3Functions.getCurrentServerConnectionHandlerID());
    }
}

PluginManager::~PluginManager() {
    gameHandler.reset();
}

void PluginManager::radioActivate(bool active) {
    if (active) {
        ts3Functions.logMessage("PluginManager activate radio", LogLevel_INFO, "GRB", ts3Functions.getCurrentServerConnectionHandlerID());
    }
    else {
        ts3Functions.logMessage("PluginManager deactivate radio", LogLevel_INFO, "GRB", ts3Functions.getCurrentServerConnectionHandlerID());
    }
}
