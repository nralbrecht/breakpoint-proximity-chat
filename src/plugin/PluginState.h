#pragma once

#include <string>

#include "ts3_functions.h"


class PluginState
{
private:
    // inline static PluginState* instance = nullptr;

    PluginState() { };
    PluginState(const PluginState&) { }; // copy constructor is private
    PluginState& operator=(const PluginState&) { return *this; }; // assignment operator is private
public:
    // static PluginState* get();

    inline static struct TS3Functions API;
    inline static std::string ID = "<unititialized>";
    inline static std::string PATH = "<unititialized>";;
};
