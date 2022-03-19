#pragma once

#include <ShlObj_core.h>
#include <Windows.h>
#include <fstream>
#include <string>

#include "nlohmann/json.hpp"
#include "Logger.h"


class PluginConfig
{
private:
    inline static const std::string CONFIG_FILE_PATH = "/grb/config.json";
    // inline static const std::string CONFIG_FILE_PATH = "C:/Users/nralbrecht/AppData/Roaming/TS3Client/plugins/grb/config.json";
    inline static PluginConfig* instance = nullptr;

    nlohmann::json config;

    PluginConfig();
    PluginConfig(const PluginConfig&) { }; // copy constructor is private
    PluginConfig& operator=(const PluginConfig&) { return *this; }; // assignment operator is private
public:
    static PluginConfig* get();

    template<typename T_target>
    T_target getValue(std::string key) {
        return config[key].get<T_target>();
    }
};
