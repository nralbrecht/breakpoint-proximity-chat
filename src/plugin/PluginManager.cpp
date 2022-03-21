#include "PluginManager.h"


PluginManager::PluginManager()
    : gameHandler(std::make_unique<GameHandler>())
    , networkManager(std::make_unique<NetworkManager>())
    , clientStateManager(std::make_unique<ClientStateManager>())
    , timer(std::make_unique<asio2::timer>())
    , radioAudioProcessor(std::make_unique<RadioAudioProcessor>())
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
            updateOwnState();
        }
        catch(const std::exception& e) {
            Logger::get()->LogF(LoggerLogLevel::Error, "error updating state: %s", e.what());
        }
    });

    networkManager->onPositionUpdate([this](DTO::ServerStateReport serverStateReport) {
        clientStateManager->onPositionUpdate(serverStateReport);
    });

    clientStateManager->onClientRadioUseChanged([this](const TS3ClientInfo& clientInfo) {
        Logger::get()->LogF(LoggerLogLevel::Verbose, "clientStateManager->onClientRadioUseChanged client changed radio use state (%s): %d", clientInfo.uuid.c_str(), clientInfo.isUsingRadio);

        if (clientInfo.isUsingRadio) {
            playWavFile("mic_click_on");
	    }
	    else {
            playWavFile("mic_click_off");
        }
    });
    clientStateManager->onClientUpdate([this](const TS3ClientInfo& clientInfo) {
        try {
            uint64 serverConnectionHandlerID = PluginState::API.getCurrentServerConnectionHandlerID();
            PluginState::API.channelset3DAttributes(serverConnectionHandlerID, clientInfo.clientId, &(clientInfo.lastKnownPosition));

            Logger::get()->LogF(LoggerLogLevel::Verbose, "clientStateManager->onClientUpdate client: '%s' pos: (%f,%f,%f) radio: %d", clientInfo.uuid.c_str(), clientInfo.lastKnownPosition.x, clientInfo.lastKnownPosition.y, clientInfo.lastKnownPosition.z, clientInfo.isUsingRadio);
        }
        catch(std::out_of_range e) { } // client not known
    });

    setWavVolume(PluginState::API.getCurrentServerConnectionHandlerID());

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
        uint64 serverConnectionHandlerID = PluginState::API.getCurrentServerConnectionHandlerID();

        if (gameHandler->isConnected() && networkManager->isConnected) {
            PluginState::API.logMessage("fetching state", LogLevel_INFO, "GRB", serverConnectionHandlerID);

            ownState = gameHandler->getState();

            PluginState::API.systemset3DListenerAttributes(serverConnectionHandlerID, &convertToVector(ownState.earPosition), &convertToVector(ownState.earForward), &convertToVector(ownState.earUp));

            int connectionStatus;
            if (PluginState::API.getConnectionStatus(serverConnectionHandlerID, &connectionStatus) != ERROR_ok) {
                PluginState::API.logMessage("cant fetch server connection state", LogLevel_ERROR, "GRB", serverConnectionHandlerID);
                throw std::runtime_error("cant fetch server connection state");
            }

            // are we connected and we have the client and channels available?
            if (connectionStatus != ConnectStatus::STATUS_CONNECTION_ESTABLISHED) {
                Logger::get()->LogF(LoggerLogLevel::Warn, "Not connected to teamspeak server: %d", connectionStatus);
                return;
            }
            Logger::get()->LogF(LoggerLogLevel::Verbose, "Connected to teamspeak server: %d", connectionStatus);

            anyID clientId = 0;
            if (PluginState::API.getClientID(serverConnectionHandlerID, &clientId) != ERROR_ok) {
                Logger::get()->LogF(LoggerLogLevel::Error, "error getClientID: %lld %d", serverConnectionHandlerID, clientId);
                throw std::runtime_error("could not get own clientId");
            }

            Logger::get()->LogF(LoggerLogLevel::Verbose, "clientID: %d", clientId);

            char* ownUUID;
            if (PluginState::API.getClientVariableAsString(serverConnectionHandlerID, clientId, ClientProperties::CLIENT_UNIQUE_IDENTIFIER, &ownUUID) != ERROR_ok) {
                Logger::get()->LogF(LoggerLogLevel::Error, "error could not get own UUID: %lld %d", serverConnectionHandlerID, clientId);
                throw std::runtime_error("could not get own UUID");
            }

            DTO::ClientState ownStateDTO;
            ownStateDTO.isUsingRadio = isUsingRadio;
            ownStateDTO.x = ownState.mouthPosition[0];
            ownStateDTO.y = ownState.mouthPosition[1];
            ownStateDTO.z = ownState.mouthPosition[2];
            ownStateDTO.uuid = std::string(ownUUID);

            PluginState::API.freeMemory(ownUUID);

            networkManager->sendPositionUpdate(ownStateDTO);
        }
        else {
            // reset all clients to 0,0 when connection to game or server is lost

            TS3_VECTOR defaultPosition;
            defaultPosition.x = 0.0f;
            defaultPosition.y = 0.0f;
            defaultPosition.z = 0.0f;

            PluginState::API.systemset3DListenerAttributes(serverConnectionHandlerID, &defaultPosition, NULL, NULL);

            for (const TS3ClientInfo* clientInfo : clientStateManager->getKnownClients()) {
                PluginState::API.channelset3DAttributes(clientInfo->serverConnectionHandlerID, clientInfo->clientId, &defaultPosition);
            }
        }
    }
    catch(const std::exception& e)
    {
        Logger::get()->LogF(LoggerLogLevel::Error, "error updateOwnState: %s", e.what());
    }
}

void PluginManager::playWavFile(std::string fileNameWithoutExtension) {
	std::string to_play = PluginState::PATH + "grb/" + fileNameWithoutExtension + ".wav";

    int error = PluginState::API.playWaveFile(PluginState::API.getCurrentServerConnectionHandlerID(), to_play.c_str());
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

void PluginManager::setWavVolume(uint64 serverConnectionHandlerID) {
    std::string radioClickVolume = std::to_string(PluginConfig::get()->getValue<int>("radio_click_volume"));
    PluginState::API.setPlaybackConfigValue(serverConnectionHandlerID, "volume_factor_wave", radioClickVolume.c_str());

    Logger::get()->LogF(LoggerLogLevel::Verbose, "PluginManager::setWavVolume set volume to '%s'", radioClickVolume.c_str());
}

void PluginManager::onCustom3dRolloffCalculationClientEvent(uint64 serverConnectionHandlerID, anyID clientID, float distance, float* volume) {
    static const float proximityRollOff = PluginConfig::get()->getValue<float>("proximity_roll_off");
    static const float proximityRollOffMaxLin = db2lin_alt2(PluginConfig::get()->getValue<float>("proximity_roll_off_max"));
    static const float proximityDistanceMin = PluginConfig::get()->getValue<float>("proximity_distance_min");
    static const float proximityDistanceMax = PluginConfig::get()->getValue<float>("proximity_distance_max");

    static const float radioRollOff = PluginConfig::get()->getValue<float>("radio_roll_off");
    static const float radioRollOffMaxLin = db2lin_alt2(PluginConfig::get()->getValue<float>("radio_roll_off_max"));
    static const float radioDistanceMin = PluginConfig::get()->getValue<float>("radio_distance_min");
    static const float radioDistanceMax = PluginConfig::get()->getValue<float>("radio_distance_max");

    try {
        auto clientInfo = clientStateManager->getClient(serverConnectionHandlerID, clientID);

        if (clientInfo.isUsingRadio) {
            *volume = 1.0f;
        }
        else {
            if ((proximityDistanceMax > 0) && (distance >= proximityDistanceMax)) {
		*volume = 0.0f;
	}
            else if (distance <= proximityDistanceMin) {
		*volume = 1.0f;
	}
	else
	{
                distance = distance - proximityDistanceMin;
		if (distance <= 1) {
			*volume = 1.0f;
		}
		else {
                    *volume = db2lin_alt2(log2(distance) * proximityRollOff);
		}

                if (*volume < proximityRollOffMaxLin) {
                    *volume = proximityRollOffMaxLin;
		}
            }
        }
    }
    catch(std::exception) {
        // ignore client info if is not known or not using radio
        *volume = 1.0f;
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

    if (newStatus == ConnectStatus::STATUS_CONNECTION_ESTABLISHED) {
        setWavVolume(serverConnectionHandlerID);
    }
}

void PluginManager::onClientMoveEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, const char* moveMessage) {
    clientStateManager->onClientMoveEvent(serverConnectionHandlerID, clientID, oldChannelID, newChannelID, visibility, moveMessage);
}

void PluginManager::onClientMoveMovedEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, anyID moverID, const char* moverName, const char* moverUniqueIdentifier, const char* moveMessage) {
    clientStateManager->onClientMoveMovedEvent(serverConnectionHandlerID, clientID, oldChannelID, newChannelID, visibility, moverID, moverName, moverUniqueIdentifier, moveMessage);
}

void PluginManager::onEditPlaybackVoiceDataEvent(uint64 serverConnectionHandlerID, anyID clientID, short* samples, int sampleCount, int channels) {
    static const float radioDestructionMin = PluginConfig::get()->getValue<float>("radio_destruction_min");
    static const float radioDestructionMax = PluginConfig::get()->getValue<float>("radio_destruction_max");

    static const float radioDistanceMin = PluginConfig::get()->getValue<float>("radio_distance_min");
    static const float radioDistanceMax = PluginConfig::get()->getValue<float>("radio_distance_max");

    try {
        TS3ClientInfo clientInfo = clientStateManager->getClient(serverConnectionHandlerID, clientID);

        if (clientInfo.isUsingRadio) {
            float destruction = 0.0f;

            if (clientInfo.isPositionKnown) {
                float distance = std::abs(std::sqrt(
                    (clientInfo.lastKnownPosition.x - ownState.earPosition[0]) * (clientInfo.lastKnownPosition.x - ownState.earPosition[0]) +
                    (clientInfo.lastKnownPosition.y - ownState.earPosition[1]) * (clientInfo.lastKnownPosition.y - ownState.earPosition[1]) +
                    (clientInfo.lastKnownPosition.z - ownState.earPosition[2]) * (clientInfo.lastKnownPosition.z - ownState.earPosition[2])
                ));


                if ((radioDistanceMax > 0) && (distance >= radioDistanceMax)) {
                    destruction = radioDestructionMax;
                }
                else if (distance <= radioDistanceMin) {
                    destruction = radioDestructionMin;
                }
                else
                {
                    distance = distance - radioDistanceMin;
                    if (distance <= 1) {
                        destruction = radioDestructionMin;
                    }
                    else {
                        destruction = ((distance / (radioDistanceMax - radioDistanceMin)) * (radioDestructionMax - radioDestructionMin)) + radioDestructionMin;
                    }

                    if (destruction > radioDestructionMax) {
                        destruction = radioDestructionMax;
                    }
                }

            }

            // Logger::get()->LogF(LoggerLogLevel::Verbose, "PluginManager::onEditPlaybackVoiceDataEvent (%d @ %lld): %d %d distance: ", clientID, serverConnectionHandlerID, sampleCount, channels, distance, );
            radioAudioProcessor->process(samples, sampleCount, channels, destruction);
        }
    }
    catch (std::exception) { } // ignore unknown clients
}
