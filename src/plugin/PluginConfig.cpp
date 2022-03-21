#include "PluginConfig.h"


PluginConfig::PluginConfig()
{
    std::ifstream configFile;
    std::string configPath = PluginState::PATH + CONFIG_FILE_PATH;
    try
    {
        configFile.open(configPath, std::ios::in);

        config = nlohmann::json::parse(configFile);

        configFile.close();
    }
    catch(const nlohmann::detail::parse_error& e)
    {
        if (configFile.is_open()) {
            configFile.close();
        }

        MessageBox(0, ("error parsing config: " + configPath).c_str(), "GRB PluginConfig", 0);
        throw e;
    }
    catch(const std::exception& e) {
        if (configFile.is_open()) {
            configFile.close();
        }

        MessageBox(0, ("could not load config file (" + std::string(e.what()) + "): " + configPath).c_str(), "GRB PluginConfig", 0);
    }
}

PluginConfig* PluginConfig::get(){
    if (instance == nullptr){
        instance = new PluginConfig();
        Logger::get()->LogF(LoggerLogLevel::Info, "Config was loaded : %s", (PluginState::PATH + CONFIG_FILE_PATH).c_str());
    }
    return instance;
}
