#include "PluginManager.h"


PluginManager::PluginManager(struct TS3Functions ts3Functions, std::string pluginId)
    : ts3Functions(ts3Functions)
    , pluginId(pluginId)
    , gameHandler(std::make_unique<GameHandler>())
    , networkManager(std::make_unique<NetworkManager>())
    , clientStateManager(std::make_unique<ClientStateManager>(ts3Functions))
    , timer(std::make_unique<asio2::timer>())
{
    timer->start_timer(PluginManager::gameHandlerReconnectTimerId, std::chrono::milliseconds(PluginConfig::get()->getValue<int>("reconnect_fequency_ms")), [&]() {
        if (!gameHandler->isConnected()) {
            try {
                Logger::get()->Log(LoggerLogLevel::Verbose, "trying to connect to game");

                gameHandler->connect();

                Logger::get()->Log(LoggerLogLevel::Verbose, "connected to game");
            }
            catch(const std::exception& e) {
                Logger::get()->LogF(LoggerLogLevel::Warn, "Was not able to connect to game: %s", e.what());
            }
        }
    });

    timer->start_timer(PluginManager::stateUpdateTimerId, std::chrono::milliseconds(PluginConfig::get()->getValue<int>("update_fequency_ms")), [&]() {
        try {
            Logger::get()->Log(LoggerLogLevel::Verbose, "updating state");

            updateOwnState();

            Logger::get()->Log(LoggerLogLevel::Verbose, "state updated");
        }
        catch(const std::exception& e) {
            Logger::get()->LogF(LoggerLogLevel::Error, "error updating state: %s", e.what());
        }
    });

    networkManager->onPositionUpdate([this](DTO::ServerStateReport serverStateReport) {
        clientStateManager->onPositionUpdate(serverStateReport);
        updatePositions();
    });

    clientStateManager->onClientRadioUseChanged([this](TS3ClientInfo clientInfo) {
        Logger::get()->LogF(LoggerLogLevel::Verbose, "PluginManager::onClientRadioUseChanged client changed radio use state (%s): %d", clientInfo.uuid.c_str(), clientInfo.isUsingRadio);

        if (clientInfo.isUsingRadio) {
            playWavFile("mic_click_on");
	    }
	    else {
            playWavFile("mic_click_off");
        }
    });

    Logger::get()->Log(LoggerLogLevel::Info, "PluginManager initialized");
}

PluginManager::~PluginManager() {
    try
    {
        Logger::get()->Log(LoggerLogLevel::Verbose, "destructing PluginManager timers");
        timer.reset();

        Logger::get()->Log(LoggerLogLevel::Verbose, "destructing PluginManager gamehandler");
        gameHandler.reset();

        Logger::get()->Log(LoggerLogLevel::Verbose, "destructing PluginManager networkmanager");
        networkManager.reset();

        Logger::get()->Log(LoggerLogLevel::Verbose, "destructed PluginManager");
    }
    catch(const std::exception& e)
    {
        Logger::get()->LogF(LoggerLogLevel::Error, "error destructing PluginManager: %s", e.what());
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

            Logger::get()->Log(LoggerLogLevel::Verbose, "Is connected to server");

            int connectionStatus;
            if (ts3Functions.getConnectionStatus(serverConnectionHandlerID, &connectionStatus) != ERROR_ok) {
                ts3Functions.logMessage("cant fetch server connection state", LogLevel_ERROR, "GRB", serverConnectionHandlerID);
                Logger::get()->Log(LoggerLogLevel::Error, "cant fetch server connection state");
                throw std::runtime_error("cant fetch server connection state");
            }

            // are we connected and we have the client and channels available?
            if (connectionStatus != ConnectStatus::STATUS_CONNECTION_ESTABLISHED) {
                Logger::get()->LogF(LoggerLogLevel::Warn, "Not connected to teamspeak server: %d", connectionStatus);
                return;
            }
            Logger::get()->LogF(LoggerLogLevel::Verbose, "Connected to teamspeak server: %d", connectionStatus);

            anyID clientId = 0;
            if (ts3Functions.getClientID(serverConnectionHandlerID, &clientId) != ERROR_ok) {
                Logger::get()->LogF(LoggerLogLevel::Error, "error getClientID: %lld %d", serverConnectionHandlerID, clientId);
                throw std::runtime_error("could not get own clientId");
            }

            Logger::get()->LogF(LoggerLogLevel::Verbose, "clientID: %d", clientId);

            char* ownUUID;
            if (ts3Functions.getClientVariableAsString(serverConnectionHandlerID, clientId, ClientProperties::CLIENT_UNIQUE_IDENTIFIER, &ownUUID) != ERROR_ok) {
                Logger::get()->LogF(LoggerLogLevel::Error, "error could not get own UUID: %lld %d", serverConnectionHandlerID, clientId);
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
            // reset all clients to 0,0 when connection to game or server is lost

            TS3_VECTOR defaultPosition;
            defaultPosition.x = 0.0f;
            defaultPosition.y = 0.0f;
            defaultPosition.z = 0.0f;

            ts3Functions.systemset3DListenerAttributes(serverConnectionHandlerID, &defaultPosition, NULL, NULL);

            for (const TS3ClientInfo* clientInfo : clientStateManager->getKnownClients()) {
                ts3Functions.channelset3DAttributes(clientInfo->serverConnectionHandlerID, clientInfo->clientId, &defaultPosition);
            }
        }
    }
    catch(const std::exception& e)
    {
        Logger::get()->LogF(LoggerLogLevel::Error, "error updateOwnState: %s", e.what());
    }
}

void PluginManager::updatePositions() {
    uint64 serverConnectionHandlerID =  ts3Functions.getCurrentServerConnectionHandlerID();

    for (const TS3ClientInfo* clientInfo : clientStateManager->getKnownClients()) {
        try {
            ts3Functions.channelset3DAttributes(serverConnectionHandlerID, clientInfo->clientId, &clientInfo->lastKnownPosition);

            Logger::get()->LogF(LoggerLogLevel::Verbose, "client: '%s' pos: (%f,%f,%f) radio: %d", clientInfo->uuid.c_str(), clientInfo->lastKnownPosition.x, clientInfo->lastKnownPosition.y, clientInfo->lastKnownPosition.z, clientInfo->isUsingRadio);
        }
        catch(std::out_of_range e) { } // client not known
    }
}

void PluginManager::playWavFile(std::string fileNameWithoutExtension) {
    char pluginPath[512];
	ts3Functions.getPluginPath(pluginPath, 512, pluginId.c_str());
	std::string path = std::string(pluginPath);

    Logger::get()->LogF(LoggerLogLevel::Verbose, "PluginManager::playWavFile plugin path %s '%s' or '%s'", pluginId.c_str(), path.c_str(), pluginPath);

	std::string to_play = path + "grb/" + fileNameWithoutExtension + ".wav";
    int error = error = ts3Functions.playWaveFile(ts3Functions.getCurrentServerConnectionHandlerID(), to_play.c_str());
    if (error == ERROR_file_not_found) {
        Logger::get()->LogF(LoggerLogLevel::Error, "PluginManager::playWavFile could not find file '%s'", to_play.c_str());
    }
	else if (error != ERROR_ok) {
		Logger::get()->LogF(LoggerLogLevel::Error, "PluginManager::playWavFile could not play (%d) '%s'", error, to_play.c_str());
	}
    else {
        Logger::get()->LogF(LoggerLogLevel::Verbose, "PluginManager::playWavFile playing '%s'", to_play.c_str());
    }
}

static inline float db2lin_alt2(float db) {
    if (db <= -200.0f) return 0.0f;
    else return exp(db/20  * log(10.0f));   // went mad with ambigous call with 10 (identified as int)
}

const float rollOff = PluginConfig::get()->getValue<float>("roll_off");
const float rollOffMaxLin = db2lin_alt2(PluginConfig::get()->getValue<float>("roll_off_max"));
const float distanceMin = PluginConfig::get()->getValue<float>("distance_min");
const float distanceMax = PluginConfig::get()->getValue<float>("distance_max");

void PluginManager::onCustom3dRolloffCalculationClientEvent(uint64 serverConnectionHandlerID, anyID clientID, float distance, float* volume) {
    try
    {
        auto clientInfo = clientStateManager->getClient(serverConnectionHandlerID, clientID);
        if (clientInfo.isUsingRadio) {
            *volume = 1.0f;
            return;
        }
    }
    catch(std::exception) { } // ignore client info if is not known or not using radio

    if ((distanceMax > 0) && (distance >= distanceMax)) {
		*volume = 0.0f;
	}
	else if (distance <= distanceMin) {
		*volume = 1.0f;
	}
	else
	{
		distance = distance - distanceMin;
		if (distance <= 1) {
			*volume = 1.0f;
		}
		else {
			*volume = db2lin_alt2(log2(distance) * rollOff);
		}

		if (*volume < rollOffMaxLin) {
			*volume = rollOffMaxLin;
		}
	}
}


void PluginManager::onHotkeyEvent(const char* keyword) {
    Logger::get()->LogF(LoggerLogLevel::Verbose, "PluginManager::onHotkeyEvent: %s", keyword);

	if (strncmp(keyword, "radio_activate", 15) == 0 && !isUsingRadio) {
        playWavFile("mic_click_on");

        isUsingRadio = true;
        Logger::get()->Log(LoggerLogLevel::Info, "PluginManager: activate radio");
	}
	else if (strncmp(keyword, "radio_deactivate", 17) == 0 && isUsingRadio){
        playWavFile("mic_click_off");

        isUsingRadio = false;
        Logger::get()->Log(LoggerLogLevel::Info, "PluginManager: deactivate radio");
	}
}

void PluginManager::onCurrentServerConnectionChanged(uint64 serverConnectionHandlerID) {
    clientStateManager->onCurrentServerConnectionChanged(serverConnectionHandlerID);
}

void PluginManager::onConnectStatusChangeEvent(uint64 serverConnectionHandlerID, int newStatus, unsigned int errorNumber) {
	clientStateManager->onConnectStatusChangeEvent(serverConnectionHandlerID, newStatus, errorNumber);
}

void PluginManager::onClientMoveEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, const char* moveMessage) {
    clientStateManager->onClientMoveEvent(serverConnectionHandlerID, clientID, oldChannelID, newChannelID, visibility, moveMessage);
}

void PluginManager::onClientMoveMovedEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, anyID moverID, const char* moverName, const char* moverUniqueIdentifier, const char* moveMessage) {
    clientStateManager->onClientMoveMovedEvent(serverConnectionHandlerID, clientID, oldChannelID, newChannelID, visibility, moverID, moverName, moverUniqueIdentifier, moveMessage);
}
