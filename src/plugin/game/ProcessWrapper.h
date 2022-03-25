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
#include <string>

#include "../Logger.h"
#include "../Vector3f.h"


class ProcessWrapper
{
private:
    DWORD pid = 0;
    HANDLE handle = nullptr;
    DWORD_PTR baseAddress = NULL;

    DWORD findPIDFromName(std::string processName);
    void openProcessReadable();
    void initBaseAddress();
    void close();
public:
    ProcessWrapper(std::string processName);
    ProcessWrapper(int pid);
    ~ProcessWrapper();

    bool isOpen();
    DWORD_PTR getBaseAddress();

    DWORD_PTR resolvePointer(DWORD_PTR address);
    DWORD_PTR resolvePointerChain(std::vector<int> offsets);

    bool readBuffer(DWORD_PTR address, LPVOID buffer, SIZE_T buffer_size);
    Vector3f readVector(DWORD_PTR address);
};
