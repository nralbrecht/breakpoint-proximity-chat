#pragma once

#include <fstream>
#include <unordered_map>
#include <optional>

#include "dto.h"
#include "sqlite3.h"


class StateManager
{
private:
    std::unordered_map<size_t, DTO::ClientState> state;
public:
    StateManager();
    ~StateManager();

    void putClientState(size_t session, DTO::ClientState clientState);
    void registerSession(size_t session);
    void clearSession(size_t session);
    DTO::ServerStateReport getStateReport();
    size_t getReportCount();
};
