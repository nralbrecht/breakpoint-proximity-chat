#include "StateManager.h"

StateManager::StateManager() {
    // dataLogFile = std::ofstream("C:/Users/nralbrecht/Desktop/positions.log", std::ios::out | std::ios::app);
}

StateManager::~StateManager() {
    // dataLogFile.flush();
    // dataLogFile.close();
}


void StateManager::putClientState(size_t session, DTO::ClientState clientState) {
    state[session] = clientState;

    // printf("uuid: %s pos: (%f, %f, %f) radio: %d\n", clientState.uuid.c_str(), clientState.x, clientState.y, clientState.z, clientState.isUsingRadio);
    // dataLogFile << clientState.uuid << "," << clientState.x << "," << clientState.y << "," << clientState.z << std::endl;
}

void StateManager::clearSession(size_t session) {
    state.erase(session);
}

DTO::ServerStateReport StateManager::getStateReport() {
    DTO::ServerStateReport serverStateReport;

    for (const auto& [_, client] : state) {
        serverStateReport.clients[client.uuid] = client;
    }

    return serverStateReport;
}
