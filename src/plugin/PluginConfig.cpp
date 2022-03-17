#include "PluginConfig.h"


PluginConfig* PluginConfig::instance = nullptr;
const std::string PluginConfig::CONFIG_FILE_PATH = "C:/Users/nralbrecht/AppData/Roaming/TS3Client/plugins/grb/grb/config.json";

PluginConfig::PluginConfig()
{
    std::ifstream configFile(CONFIG_FILE_PATH);

    configFile >> config;
}

PluginConfig* PluginConfig::get(){
    if (instance == nullptr){
        instance = new PluginConfig();
    }
    return instance;
}
