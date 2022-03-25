#pragma once

#include "ProcessWrapper.h"
#include "GameIntegration.h"


class GRBVulkan: public GameIntegration
{
private:
    std::unique_ptr<ProcessWrapper> game;

    inline static const float MAX_CAMERA_DISTANCE = 2.0f;

    DWORD_PTR healthAddress;
    DWORD_PTR identityAddress = game->getBaseAddress() + 0x5A63514;
    DWORD_PTR avatarPositionAddress = game->getBaseAddress() + 0x5B57D50;
    DWORD_PTR avatarRotationAddress;
    DWORD_PTR cameraPositionAddress = game->getBaseAddress() + 0x5B57F00;
    DWORD_PTR cameraRotationAddress = game->getBaseAddress() + 0x5B57DC0;

    // 1: can move, 0: can not
    DWORD_PTR canMoveAddress = game->getBaseAddress() + 0x5B57CE1;
    // 1: normal, 2: drone
    DWORD_PTR movementModeAddress = game->getBaseAddress() + 0x5B57CE0;
    // foot: 11, car: 1, bike: 2, tank: 6, heli: 3, plane: 8
    DWORD_PTR vehicleTypeAddress = game->getBaseAddress() + 0x56721BC;
public:
    GRBVulkan();
    ~GRBVulkan();

    bool isConnected();
    GameState getState();
};
