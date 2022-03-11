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

enum GRB_EXE {
    GRB,
    GRB_VULKAN
};
typedef enum GRB_EXE GRB_EXE_t;

struct GRB_State {
    float health;

	float avatar_position[3];
	float avatar_front[3];
    float avatar_top[3];

    float camera_position[3];
	float camera_front[3];
    float camera_top[3];
};
typedef struct GRB_State GRB_state_t;


class GameHandler
{
private:
    GRB_EXE_t executable;
    DWORD pid;
    HANDLE handle;
    DWORD_PTR base_address;

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

    void initialize();
    void shutdown();

    GRB_state_t getState();
};
