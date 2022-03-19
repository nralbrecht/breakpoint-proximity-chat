#include "PluginConfig.h"


PluginConfig::PluginConfig()
{
    std::ifstream configFile;
    try
    {
        MessageBox(0, ("test parsing config: " + std::filesystem::current_path().string() + CONFIG_FILE_PATH).c_str(), "GRB PluginConfig", 0);
        configFile.open(std::filesystem::current_path().string() + CONFIG_FILE_PATH, std::ios::in);

        config = nlohmann::json::parse(configFile);

        configFile.close();
    }
    catch(const nlohmann::detail::parse_error& e)
    {
        if (configFile.is_open()) {
            configFile.close();
        }

        MessageBox(0, ("error parsing config: " + CONFIG_FILE_PATH).c_str(), "GRB PluginConfig", 0);
        throw e;
    }
    catch(const std::exception& e) {
        MessageBox(0, ("could not load config file (" + std::string(e.what()) + "): " + CONFIG_FILE_PATH).c_str(), "GRB PluginConfig", 0);
    }
}

PluginConfig* PluginConfig::get(){
    if (instance == nullptr){
        instance = new PluginConfig();
        Logger::get()->LogF(LoggerLogLevel::Info, "Config was loaded : %s", CONFIG_FILE_PATH.c_str());
    }
    return instance;
}
