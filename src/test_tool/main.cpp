#include <cstdio>
#include <Windows.h>
#include <Libloaderapi.h>
#include <Errhandlingapi.h>
#include <fstream>
#include <algorithm>

#include "teamspeak/public_definitions.h"
#include "ts3_functions.h"

typedef int (*ts3plugin_init_t)();
typedef void (*ts3plugin_registerPluginID_t)(const char* id);
typedef void (*ts3plugin_setFunctionPointers_t)(const struct TS3Functions funcs);
typedef void (*ts3plugin_onEditPlaybackVoiceDataEvent_t)(uint64 serverConnectionHandlerID, anyID clientID, short* samples, int sampleCount, int channels); void ts3plugin_onEditPostProcessVoiceDataEvent(uint64 serverConnectionHandlerID, anyID clientID, short* samples, int sampleCount, int channels, const unsigned int* channelSpeakerArray, unsigned int* channelFillMask);

const char* pluginDir = "C:/Users/nralbrecht/AppData/Roaming/TS3Client/plugins/";

void getPluginPath(char* path, size_t maxLen, const char* pluginID) {
    strncpy_s(path, maxLen, pluginDir, 100);
};

uint64 getCurrentServerConnectionHandlerID() {
    return 1U;
}

unsigned int setPlaybackConfigValue(uint64 serverConnectionHandlerID, const char *ident, const char *value) {
    return 0U;
}

float myclamp(float min, float value, float max) {
    if (value < min) {
        return min;
    }
    else if (value > max) {
        return max;
    }
    else {
        return value;
    }
}

int main(int argc, char const *argv[])
{
    HMODULE module = LoadLibrary("C:/Users/nralbrecht/Desktop/proximity_chat/grb-ts3-plugin/build/Debug/grb_0.1.5.dll");

    if (module == NULL) {
        printf("error loading: %d", GetLastError());
    }
    else {
        printf("successfull %p", module);

        ts3plugin_init_t ts3plugin_init = (ts3plugin_init_t)GetProcAddress(module, "ts3plugin_init");
        ts3plugin_registerPluginID_t ts3plugin_registerPluginID = (ts3plugin_registerPluginID_t)GetProcAddress(module, "ts3plugin_registerPluginID");
        ts3plugin_setFunctionPointers_t ts3plugin_setFunctionPointers = (ts3plugin_setFunctionPointers_t)GetProcAddress(module, "ts3plugin_setFunctionPointers");
        ts3plugin_onEditPlaybackVoiceDataEvent_t ts3plugin_onEditPlaybackVoiceDataEvent = (ts3plugin_onEditPlaybackVoiceDataEvent_t)GetProcAddress(module, "ts3plugin_onEditPlaybackVoiceDataEvent");


        std::ifstream voicePackage("C:/Users/nralbrecht/Desktop/proximity_chat/grb-ts3-plugin/resources/samples/sample_2_480_1_312.wav", std::ios::in);
        short data[480];
        voicePackage.read((char *)(data), 480 * sizeof(short));
        voicePackage.close();

        ts3plugin_registerPluginID("{1256781283567123675}");

        struct TS3Functions funcs;
        funcs.getPluginPath = getPluginPath;
        funcs.getCurrentServerConnectionHandlerID = getCurrentServerConnectionHandlerID;
        funcs.setPlaybackConfigValue = setPlaybackConfigValue;
        ts3plugin_setFunctionPointers(funcs);

        ts3plugin_init();
        ts3plugin_onEditPlaybackVoiceDataEvent(1, 2, data, 480, 1);

        FreeLibrary(module);
    }

    return 0;
}
