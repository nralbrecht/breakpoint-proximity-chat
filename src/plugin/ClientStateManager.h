#pragma once

#include "teamspeak/public_definitions.h"
#include "teamspeak/public_errors.h"
#include "ts3_functions.h"
#include "Logger.h"
#include "dto.h"

#include <unordered_map>
#include <stdexcept>
#include <string>
#include <memory>
#include <vector>


typedef struct TS3ClientInfo {
    anyID clientId;
    uint64 serverConnectionHandlerID;
    std::string uuid;
    bool isUsingRadio;
    bool isPositionKnown;
    TS3_VECTOR lastKnownPosition;
} TS3ClientInfo;


class ClientStateManager
{
private:
    std::unordered_map<std::string, anyID> uuidToClientId;
    std::unordered_map<anyID, TS3ClientInfo> clientInfos;

    struct TS3Functions ts3Functions;
    uint64 currentServerConnectionHandlerID;

    void addNewClient(uint64 serverConnectionHandlerID, anyID clientId);
    void removeClient(anyID clientId);

    std::string getClientUUID(uint64 serverConnectionHandlerID, anyID clientId);
    void addAllCurrentClientsToMap(uint64 serverConnectionHandlerID);
public:
    ClientStateManager(struct TS3Functions ts3Functions);
    ~ClientStateManager();

    TS3ClientInfo getClient(uint64 serverConnectionHandlerID, anyID clientID);
    TS3ClientInfo getClientByUUID(std::string uuid);
    std::vector<const TS3ClientInfo*> getKnownClients();

    void onPositionUpdate(DTO::ServerStateReport serverStateReport);
    void onCurrentServerConnectionChanged(uint64 serverConnectionHandlerID);
    void onConnectStatusChangeEvent(uint64 serverConnectionHandlerID, int newStatus, unsigned int errorNumber);
    void onClientMoveEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, const char* moveMessage);
    void onClientMoveMovedEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, anyID moverID, const char* moverName, const char* moverUniqueIdentifier, const char* moveMessage);
};
