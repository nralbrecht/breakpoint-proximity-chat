#include <string>
#include <memory>

#include "GameHandler.h"
#include "ts3_functions.h"


class PluginManager
{
private:
    std::unique_ptr<GameHandler> gameHandler;
    struct TS3Functions ts3Functions;
public:
    PluginManager(struct TS3Functions ts3Functions);
    ~PluginManager();

    void radioActivate(bool active);
};
