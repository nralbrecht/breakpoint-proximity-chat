#pragma once

#include <Windows.h>
#include <processthreadsapi.h>
#include <tlhelp32.h>
#include <handleapi.h>
#include <Psapi.h>
#include <Memoryapi.h>

#include <math.h>
#include <stdio.h>
#include <stdarg.h>
#include <vector>
#include <stdexcept>

#include "../Logger.h"
#include "GameState.h"

#include "GameIntegration.h"
#include "GRB.h"
#include "GRBVulkan.h"


class GameHandler
{
private:
    std::unique_ptr<GameIntegration> game;
public:
    GameHandler();
    ~GameHandler();

    void connect();
    bool isConnected();

    GameState getState();
};
