#include "NetworkManager.h"


NetworkManager::NetworkManager()
{
    client.bind_connect([this](asio::error_code ec) {
        try
        {
            Logger::get()->Log("NetworkManager got connection: %d %s", ec.value(), ec.message().c_str());

            if (ec) {
                this->isConnected = false;
                Logger::get()->Log("NetworkManager connect failure: %d %s", asio2::last_error_val(), asio2::last_error_msg().c_str());
            }
            else {
                Logger::get()->Log("NetworkManager success");
                this->isConnected = true;
                Logger::get()->Log("%s", "NetworkManager connect success");
                // Logger::get()->Log("NetworkManager connect success: %d %s", client.local_address().c_str(), client.local_port());
            }
        }
        catch(const std::exception& e)
        {
            Logger::get()->Log("NetworkManager connect error: %s", e.what());
        }
    })
    .bind_disconnect([this](asio::error_code ec) {
        this->isConnected = false;
        Logger::get()->Log("NetworkManager disconnect: %d %s", ec.value(), ec.message().c_str());
    })
    .bind_recv([this](std::string_view sv) {
        try {
            Logger::get()->Log("NetworkManager recieved: %d %s", sv.size(), sv.data());

            DTO::ServerStateReport serverStateReport = nlohmann::json::parse(sv).get<DTO::ServerStateReport>();

            // pluginManager.updatePositions(serverStateReport);
            if (onPositionUpdateCallback) {
                onPositionUpdateCallback(serverStateReport);
            }
        }
        catch(const std::exception& e) {
            Logger::get()->Log("NetworkManager error: %s", e.what());
        }
    });

    client.async_start("89.247.153.131", 9002);
    Logger::get()->Log("%s", "NetworkManager connecting");
}

NetworkManager::~NetworkManager() {
    client.stop();
}

void NetworkManager::sendPositionUpdate(DTO::ClientState ownState) {
    Logger::get()->Log("%s", "NetworkManager serializing ClientState");
    nlohmann::json stateJson = ownState;
    Logger::get()->Log("NetworkManager sending position update: %s", stateJson.dump().c_str());

    client.async_send(stateJson.dump());
}

void NetworkManager::onPositionUpdate(std::function<void (DTO::ServerStateReport)> onPositionUpdateCallback) {
    this->onPositionUpdateCallback = onPositionUpdateCallback;
}
