#include <cstdio>
#include <Windows.h>
#include <Libloaderapi.h>
#include <Errhandlingapi.h>

int main(int argc, char const *argv[])
{
    HMODULE module = LoadLibrary("C:/Users/nralbrecht/Desktop/proximity_chat/grb-ts3-plugin/build/Debug/grb_0.1.4.dll");

    if (module == NULL) {
        printf("error loading: %d", GetLastError());
    }
    else {
        printf("successfull %p", module);
        FreeLibrary(module);
    }

    return 0;
}
