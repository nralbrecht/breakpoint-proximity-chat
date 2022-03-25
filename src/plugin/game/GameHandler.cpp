#include "GameHandler.h"


GameHandler::GameHandler()
{
    Logger::get()->Log(LoggerLogLevel::Verbose, "GameHandler init");
}

GameHandler::~GameHandler()
{
    game.reset();
}


void GameHandler::connect() {
    Logger::get()->Log(LoggerLogLevel::Info, "trying to find processes");

    try {
        game.reset(new GRB());
    }
    catch(std::exception) {
        Logger::get()->LogF(LoggerLogLevel::Info, "process not found '%s'", "GRB.exe");

        try {
            game.reset(new GRBVulkan());
        }
        catch(std::exception) {
            Logger::get()->LogF(LoggerLogLevel::Info, "process not found '%s'", "GRB_vulkan.exe");
            throw std::runtime_error("Cant find neither 'GRB.exe' nor 'GRB_vulkan.exe' to init positional audio");
        }

    }

    Logger::get()->Log(LoggerLogLevel::Info, "game initialized");
}

bool GameHandler::isConnected() {
    return game && game->isConnected();
}

// Business logic
GameState GameHandler::getState() {
    return game->getState();
}
