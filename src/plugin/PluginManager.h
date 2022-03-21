#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <fstream>

#include "teamspeak/public_definitions.h"
#include "teamspeak/public_errors.h"
#include "asio2/base/timer.hpp"
#include "ts3_functions.h"

#include "dto.h"
#include "Logger.h"
#include "VolumeUtil.h"
#include "GameHandler.h"
#include "PluginState.h"
#include "PluginConfig.h"
#include "NetworkManager.h"
#include "ClientStateManager.h"
#include "RadioAudioProcessor.h"


class PluginManager
{
private:
    std::unique_ptr<GameHandler> gameHandler;
    std::unique_ptr<NetworkManager> networkManager;
    std::unique_ptr<ClientStateManager> clientStateManager;
    std::unique_ptr<RadioAudioProcessor> radioAudioProcessor;

    std::unique_ptr<asio2::timer> timer;
    static const unsigned int stateUpdateTimerId = 0;
    static const unsigned int gameHandlerReconnectTimerId = 1;

    GRB_state_t ownState;
    bool isUsingRadio = false;
public:
    inline static const std::string PluginManager::NAME = "Tom Clancy's Ghost Recon Breakpoint";
    inline static const std::string PluginManager::VERSION = "0.1.6";
    inline static const int         PluginManager::API_VERSION = 25;
    inline static const std::string PluginManager::AUTHOR = "superananas";
    inline static const std::string PluginManager::DESCRIPTION = "Provides 3D audio and radio communication for Tom Clancy's Ghost Recon Breakpoint.";

    PluginManager();
    ~PluginManager();

    void radioActivate(bool active);
    void updateOwnState();
    void playWavFile(std::string fileNameWithoutExtension);
    void setWavVolume(uint64 serverConnectionHandlerID);

    void onHotkeyEvent(const char* keyword);
    void onCurrentServerConnectionChanged(uint64 serverConnectionHandlerID);
    void onConnectStatusChangeEvent(uint64 serverConnectionHandlerID, int newStatus, unsigned int errorNumber);
    void onClientMoveEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, const char* moveMessage);
    void onClientMoveMovedEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, anyID moverID, const char* moverName, const char* moverUniqueIdentifier, const char* moveMessage);
    void onCustom3dRolloffCalculationClientEvent(uint64 serverConnectionHandlerID, anyID clientID, float distance, float* volume);
    void onEditPlaybackVoiceDataEvent(uint64 serverConnectionHandlerID, anyID clientID, short* samples, int sampleCount, int channels);
};
