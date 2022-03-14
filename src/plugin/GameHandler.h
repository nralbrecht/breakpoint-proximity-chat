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

#include "Logger.h"


enum GRB_EXE {
    NOT_FOUND = 0,
    GRB,
    GRB_VULKAN
};
typedef enum GRB_EXE GRB_EXE_t;

struct GRB_State {
    float health;

	float mouthPosition[3];
	float mouthForward[3];
    float mouthUp[3];

    float earPosition[3];
	float earForward[3];
    float earUp[3];
};
typedef struct GRB_State GRB_state_t;


class GameHandler
{
private:
    GRB_EXE_t executable = GRB_EXE::NOT_FOUND;
    DWORD pid = 0;
    HANDLE handle = NULL;
    DWORD_PTR base_address = 0;

    DWORD getProcessByName(PCSTR name);
    void initBaseAddress();
    void openProcessReadable();
    void close();
    BOOL isOpen();
    BOOL readVector(DWORD_PTR address, LPVOID buffer, SIZE_T buffer_size);
    DWORD_PTR resolvePointer(DWORD_PTR address);
    DWORD_PTR resolvePointerChain(std::vector<int> offsets);
    void crossProduct(float* v1, float* v2, float* out);
    float vectorLength(float* vector);
    void normalize(float* vector);
    void clampDistance(float* cameraPosition, float* avatarPosition, float maxDistance, float* out);
    BOOL getErrorMessage(DWORD dwErrorCode, LPTSTR pBuffer, DWORD cchBufferLength);
    float lerp(float a, float b, float percentage);
    BOOL getGRBProcess();
public:
    GameHandler();
    ~GameHandler();

    void connect();
    bool isConnected();

    GRB_state_t getState();
};
