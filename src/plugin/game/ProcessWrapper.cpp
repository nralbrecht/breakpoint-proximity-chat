#include "ProcessWrapper.h"


ProcessWrapper::ProcessWrapper(std::string processName) {
    pid = ProcessWrapper::findPIDFromName(processName);
    if (pid == 0) {
        Logger::get()->LogF(LoggerLogLevel::Error, "Can not find process '%s' (%d)", processName.c_str(), GetLastError());

        throw std::runtime_error("Process can not be opened.");
    }

    Logger::get()->Log(LoggerLogLevel::Verbose, "opening process");
    openProcessReadable();
    if (handle == NULL) {
        Logger::get()->LogF(LoggerLogLevel::Error, "Can not open process (%d)", GetLastError());

        throw std::runtime_error("Process can not be opened.");
    }

    Logger::get()->Log(LoggerLogLevel::Verbose, "fetching process base address");
    initBaseAddress();
    if (baseAddress == 0) {
        close();

        Logger::get()->LogF(LoggerLogLevel::Error, "Not able to fetch process base address (%d)", GetLastError());

        throw std::runtime_error("Process base address can not be fetched.");
    }

    Logger::get()->Log(LoggerLogLevel::Info, "game initialized");
}

ProcessWrapper::ProcessWrapper(int pid) {
    this->pid = pid;

    Logger::get()->Log(LoggerLogLevel::Verbose, "opening process");
    openProcessReadable();
    if (handle == NULL) {
        Logger::get()->LogF(LoggerLogLevel::Error, "Can not open process (%d)", GetLastError());

        throw std::runtime_error("Process can not be opened.");
    }

    Logger::get()->Log(LoggerLogLevel::Verbose, "fetching process base address");
    initBaseAddress();
    if (baseAddress == 0) {
        close();

        Logger::get()->LogF(LoggerLogLevel::Error, "Not able to fetch process base address (%d)", GetLastError());

        throw std::runtime_error("Process base address can not be fetched.");
    }

    Logger::get()->Log(LoggerLogLevel::Info, "game initialized");
}

ProcessWrapper::~ProcessWrapper() {
    close();
}

DWORD ProcessWrapper::findPIDFromName(std::string processName) {
    // Create toolhelp snapshot.
    DWORD pid = 0;
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
            if (strncmp(process.szExeFile, processName.c_str(), 124) == 0) {
                pid = process.th32ProcessID;
                break;
            }
        } while (Process32Next(snapshot, &process));
    }

    CloseHandle(snapshot);

    return pid;
}


void ProcessWrapper::initBaseAddress() {
    baseAddress = 0;
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
}

void ProcessWrapper::openProcessReadable() {
    handle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
}

void ProcessWrapper::close() {
    if (handle != NULL) {
        if (CloseHandle(handle) == 0) {
            Logger::get()->LogF(LoggerLogLevel::Error, "error closing handle %p: %d", handle, GetLastError());
            handle = NULL;
        }
        else {
            // successfully closed
            handle = NULL;
        }
    }
}

bool ProcessWrapper::isOpen() {
    if (pid == 0 || handle == nullptr || baseAddress == NULL) {
        return false;
    }

    DWORD exitCode = 0;
    GetExitCodeProcess(handle, &exitCode);

    return exitCode == STILL_ACTIVE;
}

bool ProcessWrapper::readBuffer(DWORD_PTR address, LPVOID buffer, SIZE_T buffer_size) {
    return ReadProcessMemory(handle, (LPCVOID)(address), buffer, buffer_size, NULL);
}

Vector3f ProcessWrapper::readVector(DWORD_PTR address) {
    float tempVectorBuffer[3];

    readBuffer(address, static_cast<LPVOID>(tempVectorBuffer), 3 * sizeof(float));

    return Vector3f(tempVectorBuffer[0], tempVectorBuffer[1], tempVectorBuffer[2]);
}

DWORD_PTR ProcessWrapper::resolvePointer(DWORD_PTR address) {
    DWORD_PTR output = 0;
    readBuffer(address, (LPVOID*)(&output), sizeof(DWORD_PTR));

    return output;
}

DWORD_PTR ProcessWrapper::resolvePointerChain(std::vector<int> offsets) {
    if (offsets.size() == 0) {
        return baseAddress;
    }

    DWORD_PTR address = resolvePointer(baseAddress + offsets[0]);

    if (offsets.size() == 1) {
        return address;
    }

    for (int i = 1; i < (offsets.size() - 1); i++) {
        address = resolvePointer(address + offsets[i]);
    }

    return address + offsets[offsets.size() - 1];
}

DWORD_PTR ProcessWrapper::getBaseAddress() {
    return baseAddress;
}
