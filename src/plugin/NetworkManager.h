#pragma once

#include <functional>

#include "asio2/tcp/tcp_client.hpp"
#include "nlohmann/json.hpp"
#include "GameHandler.h"
#include "Logger.h"
#include "dto.h"


class NetworkManager
{
private:
    asio2::tcp_client client;
    std::function<void (DTO::ServerStateReport)> onPositionUpdateCallback;
public:
    NetworkManager();
    ~NetworkManager();

    bool isConnected = false;

    void sendPositionUpdate(DTO::ClientState ownState);

    void onPositionUpdate(std::function<void (DTO::ServerStateReport)> onPositionUpdateCallback);
};
