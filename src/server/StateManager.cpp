#include "StateManager.h"


StateManager::StateManager() { }
StateManager::~StateManager() { }

void StateManager::putClientState(size_t session, DTO::ClientState clientState) {
    state[session] = clientState;
}

void StateManager::clearSession(size_t session) {
    state.erase(session);
}

DTO::ServerStateReport StateManager::getStateReport() {
    DTO::ServerStateReport serverStateReport;

    for (const auto& [_, client] : state) {
        serverStateReport.push_back(client);
    }

    return serverStateReport;
}

size_t StateManager::getReportCount() {
    return state.size();
}
