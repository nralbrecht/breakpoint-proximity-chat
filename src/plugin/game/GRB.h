#pragma once

#include "ProcessWrapper.h"
#include "GameIntegration.h"


class GRB: public GameIntegration
{
private:
    std::unique_ptr<ProcessWrapper> game;

    inline static const float MAX_CAMERA_DISTANCE = 2.0f;

    DWORD_PTR healthAddress;
    DWORD_PTR identityAddress;
    DWORD_PTR avatarPositionAddress;
    DWORD_PTR avatarRotationAddress;
    DWORD_PTR cameraPositionAddress;
    DWORD_PTR cameraRotationAddress;

    // 1: can move, 0: can not
    DWORD_PTR canMoveAddress;
    // 1: normal, 2: drone
    DWORD_PTR movementModeAddress;
    // foot: 11, car: 1, bike: 2, tank: 6, heli: 3, plane: 8
    DWORD_PTR vehicleTypeAddress;
public:
    GRB();
    ~GRB();

    bool isConnected();
    GameState getState();
};
