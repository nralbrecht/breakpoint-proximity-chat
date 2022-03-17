#include "ClientStateManager.h"


ClientStateManager::ClientStateManager(struct TS3Functions ts3Functions)
    : ts3Functions(ts3Functions)
	, currentServerConnectionHandlerID(ts3Functions.getCurrentServerConnectionHandlerID())
{
}

ClientStateManager::~ClientStateManager() {
}

TS3ClientInfo ClientStateManager::getClient(uint64 serverConnectionHandlerID, anyID clientID) {
	return clientInfos[clientID];
}

TS3ClientInfo ClientStateManager::getClientByUUID(std::string uuid) {
    return clientInfos[uuidToClientId[uuid]];
}

std::vector<const TS3ClientInfo*> ClientStateManager::getKnownClients() {
	std::vector<const TS3ClientInfo*> result;

	for (auto [clientId, clientInfo] : clientInfos) {
		if (clientInfo.isPositionKnown) {
			result.push_back(&clientInfo);
		}
	}

	return result;
}

std::string ClientStateManager::getClientUUID(uint64 serverConnectionHandlerID, anyID clientId) {
	char* uuidBuffer;

	unsigned int errorCode = ts3Functions.getClientVariableAsString(serverConnectionHandlerID, clientId, ClientProperties::CLIENT_UNIQUE_IDENTIFIER, &uuidBuffer);
	if (errorCode != ERROR_ok) {
		Logger::get()->LogF(LoggerLogLevel::Error, "could not get UUID for server %lld and client %d. Error %d", serverConnectionHandlerID, clientId, errorCode);
		throw std::runtime_error("could not get UUID");
	}

	std::string uuid(uuidBuffer);
	ts3Functions.freeMemory(uuidBuffer);

	return uuid;
}

void ClientStateManager::addNewClient(uint64 serverConnectionHandlerID, anyID clientId) {
	if (clientInfos.count(clientId) != 0) {
		return;
	}

	TS3ClientInfo clientInfo;
	clientInfo.clientId = clientId;
	clientInfo.serverConnectionHandlerID = serverConnectionHandlerID;
	clientInfo.uuid = getClientUUID(serverConnectionHandlerID, clientId);
	clientInfo.isPositionKnown = false;

	clientInfos[clientInfo.clientId] = clientInfo;
	uuidToClientId[clientInfo.uuid] = clientInfo.clientId;
}

void ClientStateManager::removeClient(anyID clientId) {
	try
	{
		uuidToClientId.erase(clientInfos.at(clientId).uuid);
		clientInfos.erase(clientId);
	}
	catch(const std::out_of_range& e)
	{
		Logger::get()->LogF(LoggerLogLevel::Warn, "was not able to remove client from storage %d", e.what());
	}
}

void ClientStateManager::addAllCurrentClientsToMap(uint64 serverConnectionHandlerID) {
	int connectionStatus;
	if (ts3Functions.getConnectionStatus(serverConnectionHandlerID, &connectionStatus) != ERROR_ok) {
		Logger::get()->Log(LoggerLogLevel::Error, "ClientStateManager::addAllCurrentClientsToMap error could not get connection status");
		return;
	}
	if (connectionStatus != ConnectStatus::STATUS_CONNECTION_ESTABLISHED) {
		return;
	}

	anyID* clientList;
	if(ts3Functions.getClientList(serverConnectionHandlerID, &clientList) != ERROR_ok) {
		Logger::get()->LogF(LoggerLogLevel::Error, "ClientStateManager::addAllCurrentClientsToMap error could not get client list: %lld", serverConnectionHandlerID);
		return;
	}

	for (size_t i = 0; true; i++) {
		anyID clientId = clientList[i];

		if (clientId == 0) {
			break;
		}
		else {
			// not yet known user
			addNewClient(serverConnectionHandlerID, clientId);
		}
	}

	ts3Functions.freeMemory(clientList);
	Logger::get()->Log(LoggerLogLevel::Verbose, "ClientStateManager::addAllCurrentClientsToMap map");
}

void ClientStateManager::onPositionUpdate(DTO::ServerStateReport serverStateReport) {
    for (const DTO::ClientState& client : serverStateReport) {
        try {
            anyID otherClientId = uuidToClientId.at(client.uuid);

            TS3_VECTOR otherPosition;
			otherPosition.x = client.x;
			otherPosition.y = client.y;
			otherPosition.z = client.z;

			clientInfos[otherClientId].lastKnownPosition = otherPosition;
			clientInfos[otherClientId].isPositionKnown = true;
			clientInfos[otherClientId].isUsingRadio = client.isUsingRadio;

            Logger::get()->LogF(LoggerLogLevel::Verbose, "client: '%s' pos: (%f,%f,%f) radio: %d", client.uuid.c_str(), client.x, client.y, client.z, client.isUsingRadio);
        }
        catch(std::out_of_range e) { } // ignore clients not known by the plugin host
    }
}

void ClientStateManager::onCurrentServerConnectionChanged(uint64 serverConnectionHandlerID) {
	currentServerConnectionHandlerID = serverConnectionHandlerID;
    Logger::get()->LogF(LoggerLogLevel::Info, "ClientStateManager::currentServerConnectionChanged: %lld", serverConnectionHandlerID);
}

void ClientStateManager::onConnectStatusChangeEvent(uint64 serverConnectionHandlerID, int newStatus, unsigned int errorNumber) {
    Logger::get()->LogF(LoggerLogLevel::Verbose, "ClientStateManager::onConnectStatusChangeEvent: %lld %d %d", serverConnectionHandlerID, newStatus, errorNumber);

	if (newStatus == ConnectStatus::STATUS_CONNECTION_ESTABLISHED) {
		addAllCurrentClientsToMap(serverConnectionHandlerID);
	}
	else if (newStatus == ConnectStatus::STATUS_DISCONNECTED) {
		uuidToClientId.clear();
		clientInfos.clear();

		Logger::get()->Log(LoggerLogLevel::Verbose, "ClientStateManager::onConnectStatusChangeEvent disconected map");
	}
}

void ClientStateManager::onClientMoveEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, const char* moveMessage) {
	Logger::get()->LogF(LoggerLogLevel::Verbose, "ClientStateManager::onClientMoveEvent: %lld %d %lld -> %lld %d '%s'", serverConnectionHandlerID, clientID, oldChannelID, newChannelID, visibility, moveMessage);
	if (oldChannelID == 0) {
		// joined
		addNewClient(serverConnectionHandlerID, clientID);
		Logger::get()->LogF(LoggerLogLevel::Verbose, "ClientStateManager::onClientMoveEvent added client %d", clientID);
	}
	else if (newChannelID == 0) {
		// left
		removeClient(clientID);
		Logger::get()->LogF(LoggerLogLevel::Verbose, "ClientStateManager::onClientMoveEvent removed client %d", clientID);
	}
}

void ClientStateManager::onClientMoveMovedEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, anyID moverID, const char* moverName, const char* moverUniqueIdentifier, const char* moveMessage) {
	// TODO: handle moved by server/other client
    Logger::get()->LogF(LoggerLogLevel::Verbose, "ClientStateManager::onClientMoveMovedEvent: %lld %d %lld -> %lld %d %d '%d' '%s' '%s'", serverConnectionHandlerID, clientID, oldChannelID, newChannelID, visibility, moverID, moverName, moverUniqueIdentifier, moveMessage);
}
