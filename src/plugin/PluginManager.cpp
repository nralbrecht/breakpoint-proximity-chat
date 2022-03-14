#include "PluginManager.h"


PluginManager::PluginManager(struct TS3Functions ts3Functions, const std::unordered_map<std::string, anyID> &uuidForAvailableClients)
    : ts3Functions(ts3Functions)
    , uuidForAvailableClients(uuidForAvailableClients)
    , gameHandler(std::make_unique<GameHandler>())
    , networkManager(std::make_unique<NetworkManager>())
    , timer(std::make_unique<asio2::timer>())
{
    timer->start_timer(PluginManager::gameHandlerReconnectTimerId, std::chrono::milliseconds(2000), [&]() {
        if (!gameHandler->isConnected()) {
            try {
                Logger::get()->Log("%s", "trying to connect to game");

                gameHandler->connect();

                Logger::get()->Log("%s", "connected to game");
            }
            catch(const std::exception& e) {
                Logger::get()->Log("was not able to connect to game: %s", e.what());
            }
        }
    });

    timer->start_timer(PluginManager::stateUpdateTimerId, std::chrono::milliseconds(250), [&]() {
        try {
            Logger::get()->Log("%s", "updating state");
            updateOwnState();
            Logger::get()->Log("%s", "state updated");
        }
        catch(const std::exception& e) {
            Logger::get()->Log("error updating state: %s", e.what());
        }
    });

    networkManager->onPositionUpdate([this](DTO::ServerStateReport serverStateReport) {
        updatePositions(serverStateReport);
    });

    Logger::get()->Log("%s", "PluginManager initialized");
}

PluginManager::~PluginManager() {
    try
    {
        Logger::get()->Log("%s", "destructing PluginManager timers");
        timer.reset();

        Logger::get()->Log("%s", "destructing PluginManager gamehandler");
        gameHandler.reset();

        Logger::get()->Log("%s", "destructing PluginManager networkmanager");
        networkManager.reset();

        Logger::get()->Log("%s", "destructed PluginManager");
    }
    catch(const std::exception& e)
    {
        Logger::get()->Log("error destructing PluginManager: %s", e.what());
    }
}


void PluginManager::radioActivate(bool active) {
    if (isUsingRadio == active) {
        return;
    }

    if (active) {
        isUsingRadio = true;
        ts3Functions.logMessage("PluginManager activate radio", LogLevel_INFO, "GRB", ts3Functions.getCurrentServerConnectionHandlerID());
    }
    else {
        isUsingRadio = false;
        ts3Functions.logMessage("PluginManager deactivate radio", LogLevel_INFO, "GRB", ts3Functions.getCurrentServerConnectionHandlerID());
    }
}

TS3_VECTOR convertToVector(float *vector3) {
    TS3_VECTOR vector;

    vector.x = vector3[0];
    vector.y = vector3[1];
    vector.z = vector3[2];

    return vector;
}

void PluginManager::updateOwnState() {
    try
    {
        uint64 serverConnectionHandlerID = ts3Functions.getCurrentServerConnectionHandlerID();

        if (gameHandler->isConnected() && networkManager->isConnected) {
            ts3Functions.logMessage("fetching state", LogLevel_INFO, "GRB", serverConnectionHandlerID);

            ownState = gameHandler->getState();

            ts3Functions.systemset3DListenerAttributes(serverConnectionHandlerID, &convertToVector(ownState.earPosition), &convertToVector(ownState.earForward), &convertToVector(ownState.earUp));

            Logger::get()->Log("%s", "Is connected to server");

            int connectionStatus;
            if (ts3Functions.getConnectionStatus(serverConnectionHandlerID, &connectionStatus) != ERROR_ok) {
                ts3Functions.logMessage("cant fetch server connection state", LogLevel_ERROR, "GRB", serverConnectionHandlerID);
                throw std::runtime_error("not connected to a server");
            }

            // are we connected and we have the client and channels available?
            if (connectionStatus != ConnectStatus::STATUS_CONNECTION_ESTABLISHED) {
                Logger::get()->Log("Not connected to teamspeak server: %d", connectionStatus);
                return;
            }
            Logger::get()->Log("Connected to teamspeak server: %d", connectionStatus);

            // ts3Functions.channelset3DAttributes(serverConnectionHandlerID,

            anyID clientId = 0;
            if (ts3Functions.getClientID(serverConnectionHandlerID, &clientId) != ERROR_ok) {
                Logger::get()->Log("error getClientID: %lld %p", serverConnectionHandlerID, clientId);
                throw std::runtime_error("could not get own clientId");
            }

            Logger::get()->Log("clientID: %d", clientId);

            char* ownUUID;
            if (ts3Functions.getClientVariableAsString(serverConnectionHandlerID, clientId, ClientProperties::CLIENT_UNIQUE_IDENTIFIER, &ownUUID) != ERROR_ok) {
                throw std::runtime_error("could not get own UUID");
            }

            DTO::ClientState ownStateDTO;
            ownStateDTO.isUsingRadio = isUsingRadio;
            ownStateDTO.x = ownState.mouthPosition[0];
            ownStateDTO.y = ownState.mouthPosition[1];
            ownStateDTO.z = ownState.mouthPosition[2];
            ownStateDTO.uuid = std::string(ownUUID);

            ts3Functions.freeMemory(ownUUID);

            networkManager->sendPositionUpdate(ownStateDTO);
        }
        else {
            TS3_VECTOR defaultPosition;
            defaultPosition.x = 0.0f;
            defaultPosition.y = 0.0f;
            defaultPosition.z = 0.0f;

            ts3Functions.systemset3DListenerAttributes(serverConnectionHandlerID, &defaultPosition, NULL, NULL);

            for (const auto& [uuid, clientId] : uuidForAvailableClients) {
                ts3Functions.channelset3DAttributes(serverConnectionHandlerID, clientId, &defaultPosition);
            }
        }
    }
    catch(char * e) {
        Logger::get()->Log("error updateOwnState auto: %s", e);
    }
    catch(const std::exception& e)
    {
        Logger::get()->Log("error updateOwnState: %s", e.what());
    }
}

void PluginManager::updatePositions(DTO::ServerStateReport serverStateReport) {
    uint64 serverConnectionHandlerID =  ts3Functions.getCurrentServerConnectionHandlerID();
    for (const auto& [uuid, client] : serverStateReport.clients) {
        try {
            anyID otherClientId = uuidForAvailableClients.at(uuid);

            TS3_VECTOR otherPosition;
            if (client.isUsingRadio) {
                otherPosition.x = ownState.earPosition[0];
                otherPosition.y = ownState.earPosition[1];
                otherPosition.z = ownState.earPosition[2];
            }
            else {
                otherPosition.x = client.x;
                otherPosition.y = client.y;
                otherPosition.z = client.z;
            }

            ts3Functions.channelset3DAttributes(serverConnectionHandlerID, otherClientId, &otherPosition);

            Logger::get()->Log("client: '%s' pos: (%f,%f,%f) radio: %d", uuid.c_str(), client.x, client.y, client.z, client.isUsingRadio);
        }
        catch(std::out_of_range e) { } // client not known
    }
}
