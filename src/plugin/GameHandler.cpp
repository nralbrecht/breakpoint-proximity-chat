#include "GameHandler.h"


GameHandler::GameHandler()
{
    SetLastError(0);

    pid = getProcessByName("GRB.exe");
    if (pid == 0) {
        pid = getProcessByName("GRB_vulkan.exe");
        if (pid == 0) {
            throw std::runtime_error("Cant find neither 'GRB.exe' nor 'GRB_vulkan.exe' to init positional audio");
        }
        else {
            executable = GRB_VULKAN;
        }
    }
    else {
        executable = GRB;
    }

    openProcessReadable();
    if (handle == 0) {
        throw std::runtime_error("Process can not be opened.");
    }

    initBaseAddress();
    if (base_address == 0) {
        close();
        throw std::runtime_error("Process base address can not be fetched.");
    }
}

GameHandler::~GameHandler()
{
    close();
}


// Helper Functions
DWORD GameHandler::getProcessByName(PCSTR name) {
    DWORD pid = 0;

    // Create toolhelp snapshot.
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 process;
    ZeroMemory(&process, sizeof(process));
    process.dwSize = sizeof(process);

    SetLastError(0);

    // Walkthrough all processes.
    if (Process32First(snapshot, &process)) {
        do {
            // Compare process.szExeFile based on format of name, i.e., trim file path
            // trim .exe if necessary, etc.
            if (strncmp(process.szExeFile, name, 18) == 0)
            {
                pid = process.th32ProcessID;
                break;
            }
        } while (Process32Next(snapshot, &process));
    }

    CloseHandle(snapshot);

    return pid;
}

void GameHandler::initBaseAddress() {
    DWORD_PTR   baseAddress = 0;
    HMODULE*    moduleArray;
    LPBYTE      moduleArrayBytes;
    DWORD       bytesRequired;

    SetLastError(0);

    if (EnumProcessModulesEx(handle, NULL, 0, &bytesRequired, LIST_MODULES_64BIT)) {
        if (bytesRequired) {
            moduleArrayBytes = (LPBYTE)LocalAlloc(LPTR, bytesRequired);

            if (moduleArrayBytes) {
                unsigned int moduleCount;

                moduleCount = bytesRequired / sizeof(HMODULE);
                moduleArray = (HMODULE*)moduleArrayBytes;

                if (EnumProcessModulesEx(handle, moduleArray, bytesRequired, &bytesRequired, LIST_MODULES_64BIT)) {
                    baseAddress = (DWORD_PTR)moduleArray[0];
                }

                LocalFree(moduleArrayBytes);
            }
        }
    }

    base_address = baseAddress;
}

void GameHandler::openProcessReadable() {
    handle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
}

void GameHandler::close() {
    if (CloseHandle(handle)) {
        handle = NULL;
    }
    else {
        throw std::runtime_error("Could not close process handle.");
    }
}

BOOL GameHandler::isOpen() {
    DWORD exitCode = 0;

    GetExitCodeProcess(handle, &exitCode);

    return exitCode == STILL_ACTIVE;
}

BOOL GameHandler::readVector(DWORD_PTR address, LPVOID buffer, SIZE_T buffer_size) {
    return ReadProcessMemory(handle, (LPCVOID)(address), buffer, buffer_size, NULL);
}

DWORD_PTR GameHandler::resolvePointer(DWORD_PTR address) {
    DWORD_PTR output = 0;
    readVector(address, (LPVOID*)(&output), sizeof(DWORD_PTR));

    return output;
}

DWORD_PTR GameHandler::resolvePointerChain(std::vector<int> offsets) {
    if (offsets.size() == 0) {
        return base_address;
    }

    DWORD_PTR address = resolvePointer(base_address + offsets[0]);

    if (offsets.size() == 1) {
        return address;
    }

    for (int i = 1; i < (offsets.size() - 1); i++) {
        address = resolvePointer(address + offsets[i]);
    }

    return address + offsets[offsets.size() - 1];
}

void GameHandler::crossProduct(float* v1, float* v2, float* out) {
    out[0] = v1[1] * v2[2] - v1[2] * v2[1];
    out[1] = v1[2] * v2[0] - v1[0] * v2[2];
    out[2] = v1[0] * v2[1] - v1[1] * v2[0];
}

float GameHandler::vectorLength(float* vector) {
    return sqrtf(
        (vector[0] * vector[0]) +
        (vector[1] * vector[1]) +
        (vector[2] * vector[2])
    );
}

void GameHandler::normalize(float* vector) {
    float length = vectorLength(vector);

    vector[0] = vector[0] / length;
    vector[1] = vector[1] / length;
    vector[2] = vector[2] / length;
}

void GameHandler::clampDistance(float* cameraPosition, float* avatarPosition, float maxDistance, float* out) {
    float avatarToCamera[3];
    avatarToCamera[0] = cameraPosition[0] - avatarPosition[0];
    avatarToCamera[1] = cameraPosition[1] - avatarPosition[1];
    avatarToCamera[2] = cameraPosition[2] - avatarPosition[2];

    if (vectorLength(avatarToCamera) > maxDistance) {
        normalize(avatarToCamera);

        out[0] = avatarPosition[0] + (avatarToCamera[0] * maxDistance);
        out[1] = avatarPosition[1] + (avatarToCamera[1] * maxDistance);
        out[2] = avatarPosition[2] + (avatarToCamera[2] * maxDistance);
    }
    else {
        out[0] = cameraPosition[0];
        out[1] = cameraPosition[1];
        out[2] = cameraPosition[2];
    }
}

// This functions fills a caller-defined character buffer (pBuffer)
// of max length (cchBufferLength) with the human-readable error message
// for a Win32 error code (dwErrorCode).
//
// Returns TRUE if successful, or FALSE otherwise.
// If successful, pBuffer is guaranteed to be NUL-terminated.
// On failure, the contents of pBuffer are undefined.
BOOL GameHandler::getErrorMessage(DWORD dwErrorCode, LPTSTR pBuffer, DWORD cchBufferLength) {
    if (cchBufferLength == 0) {
        return FALSE;
    }

    DWORD cchMsg = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,  /* (not used with FORMAT_MESSAGE_FROM_SYSTEM) */
        dwErrorCode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        pBuffer,
        cchBufferLength,
        NULL);

    return (cchMsg > 0);
}

float GameHandler::lerp(float a, float b, float percentage) {
    return (a * percentage) + (b * (1.0f - percentage));
}

// Business logic
GRB_state_t GameHandler::getState() {
    GRB_state_t state;

    // get up to date pointers
    DWORD_PTR health_address;
	// DWORD_PTR identity_address;
	DWORD_PTR avatar_position_address;
	DWORD_PTR avatar_rotation_address;
	DWORD_PTR camera_position_address;
	DWORD_PTR camera_rotation_address;

    if (executable == GRB) {
        health_address = resolvePointerChain({ 0x05B5FE88, 0x0, 0x4C0, 0x998 });
		// identity_address = base_address + 0x5A63514;

        avatar_position_address = base_address + 0x5B57D50;
		avatar_rotation_address = resolvePointerChain({ 0x05B579B8, 0x90, 0x0, 0x28, 0xB0 });

        camera_position_address = base_address + 0x5B57F00;
		camera_rotation_address = base_address + 0x5B57DC0;
	}
	else if (executable == GRB_VULKAN) {
		health_address = resolvePointerChain({ 0x063EDFEC, 0x38, 0xD0, 0x190, 0x8 });
		// identity_address = base_address + 0x62F2844;

        avatar_position_address = base_address + 0x63E7230;
		avatar_rotation_address = resolvePointerChain({ 0x06219AA0, 0x40, 0xD38 });

        camera_position_address = base_address + 0x63E7080;
		camera_rotation_address = base_address + 0x63E70F0;
	}

    // fetch and preprocess data
    // health
    readVector(health_address, (LPVOID)&(state.health), sizeof(FLOAT));

	// avatar position
	float avatar_pos_corrector[3];

	readVector(avatar_position_address, (LPVOID)avatar_pos_corrector, sizeof(float) * 3);

	state.avatar_position[0] = avatar_pos_corrector[0];
	state.avatar_position[1] = avatar_pos_corrector[2];
	state.avatar_position[2] = avatar_pos_corrector[1];

    // avatar front
	float avatar_rot_corrector[2];
	readVector(avatar_rotation_address, (LPVOID)avatar_rot_corrector, sizeof(float) * 2);

	state.avatar_front[0] = avatar_rot_corrector[0];
	state.avatar_front[1] = 0.0f;
	state.avatar_front[2] = avatar_rot_corrector[1];
    normalize(state.avatar_front);

    // avatar top
	state.avatar_top[0] = 0.0f;
	state.avatar_top[1] = 1.0f;
	state.avatar_top[2] = 0.0f;

	// camera position
	float camera_pos_corrector[3];
	readVector(camera_position_address, (LPVOID*)camera_pos_corrector, sizeof(float) * 3);

	state.camera_position[0] = lerp(camera_pos_corrector[0], avatar_pos_corrector[0], 0.5f);
	state.camera_position[1] = lerp(camera_pos_corrector[2], avatar_pos_corrector[2], 0.5f);
	state.camera_position[2] = lerp(camera_pos_corrector[1], avatar_pos_corrector[1], 0.5f);

    clampDistance(state.camera_position, state.avatar_position, 2.0f, state.camera_position);

    // camera front
	float camera_front_corrector[3];
	readVector(camera_rotation_address, (LPVOID*)camera_front_corrector, sizeof(float) * 3);

	state.camera_front[0] = camera_front_corrector[0];
	state.camera_front[1] = camera_front_corrector[2];
	state.camera_front[2] = camera_front_corrector[1];
    normalize(state.camera_front);

    // camera top
	float camera_right_corrector[3];
	crossProduct(state.camera_front, state.avatar_top, camera_right_corrector);

	float camera_top_corrector[3];
	crossProduct(camera_right_corrector, state.camera_front, camera_top_corrector);
	normalize(camera_top_corrector);

    state.camera_top[0] = camera_top_corrector[0];
	state.camera_top[1] = camera_top_corrector[1];
	state.camera_top[2] = camera_top_corrector[2];

    return state;
}
