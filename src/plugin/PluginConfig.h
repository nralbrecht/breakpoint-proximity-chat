#pragma once

#include <fstream>
#include <string>

#include "nlohmann/json.hpp"


class PluginConfig
{
private:
    static const std::string CONFIG_FILE_PATH;
    static PluginConfig* instance;

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
