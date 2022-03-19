#include "NetworkManager.h"


NetworkManager::NetworkManager()
{
    client.bind_connect([this](asio::error_code ec) {
        try
        {
            Logger::get()->LogF(LoggerLogLevel::Verbose, "NetworkManager got connection: %d %s", ec.value(), ec.message().c_str());

            if (ec) {
                this->isConnected = false;
                Logger::get()->LogF(LoggerLogLevel::Warn, "NetworkManager connect failure: %d %s", asio2::last_error_val(), asio2::last_error_msg().c_str());
            }
            else {
                Logger::get()->Log(LoggerLogLevel::Verbose, "NetworkManager success");
                this->isConnected = true;
                Logger::get()->Log(LoggerLogLevel::Info, "NetworkManager connect success");
                // Logger::get()->Log("NetworkManager connect success: %d %s", client.local_address().c_str(), client.local_port());
            }
        }
        catch(const std::exception& e)
        {
            Logger::get()->LogF(LoggerLogLevel::Error, "NetworkManager connect error: %s", e.what());
        }
    })
    .bind_disconnect([this](asio::error_code ec) {
        this->isConnected = false;
        Logger::get()->LogF(LoggerLogLevel::Info, "NetworkManager disconnect: %d %s", ec.value(), ec.message().c_str());
    })
    .bind_recv([this](std::string_view sv) {
        try {
            Logger::get()->LogF(LoggerLogLevel::Info, "NetworkManager recieved: %d %s", sv.size(), sv.data());

            DTO::ServerStateReport serverStateReport = nlohmann::json::parse(sv).get<DTO::ServerStateReport>();

            if (onPositionUpdateCallback) {
                onPositionUpdateCallback(serverStateReport);
            }
        }
        catch(const std::exception& e) {
            Logger::get()->LogF(LoggerLogLevel::Error, "NetworkManager error: %s", e.what());
        }
    });

    client.async_start(PluginConfig::get()->getValue<std::string>("server_host"), PluginConfig::get()->getValue<int>("server_port"));
    Logger::get()->LogF(LoggerLogLevel::Info, "NetworkManager connecting to '%s:%d'", PluginConfig::get()->getValue<std::string>("server_host").c_str(), PluginConfig::get()->getValue<int>("server_port"));
}

NetworkManager::~NetworkManager() {
    client.stop();
}

void NetworkManager::sendPositionUpdate(DTO::ClientState ownState) {
    Logger::get()->Log(LoggerLogLevel::Verbose, "NetworkManager serializing ClientState");
    nlohmann::json stateJson = ownState;
    Logger::get()->LogF(LoggerLogLevel::Verbose, "NetworkManager sending position update: %s", stateJson.dump().c_str());

    client.async_send(stateJson.dump());
}

void NetworkManager::onPositionUpdate(std::function<void (DTO::ServerStateReport)> onPositionUpdateCallback) {
    this->onPositionUpdateCallback = onPositionUpdateCallback;
}
