#pragma once

#include "GameState.h"

class GameIntegration
{
public:
    virtual bool isConnected() = 0;
    virtual GameState getState() = 0;
};
