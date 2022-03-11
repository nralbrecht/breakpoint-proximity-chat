cmake_minimum_required (VERSION 3.17)

set(PLUGIN_VERSION "0.1.0")

project(GRBTS3Plugin
    VERSION "${PLUGIN_VERSION}"
    DESCRIPTION "Tom Clancy's Ghost Recon: Breakpoint"
    LANGUAGES "CXX"
)

include(ExternalProject)

ExternalProject_Add(pluginsdk
    URL http://dl.4players.de/ts/client/pluginsdk/pluginsdk_3.1.1.1.zip
    PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/deps/pluginsdk
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
)

# plugin
add_library(ts3plugin
    SHARED
    src/plugin/main.cpp
    src/plugin/GameHandler.cpp
    src/plugin/PluginManager.cpp
)

target_include_directories(ts3plugin
    PUBLIC "${CMAKE_SOURCE_DIR}/include"
	PUBLIC "${CMAKE_SOURCE_DIR}/deps/pluginsdk/src/pluginsdk/include"
    PUBLIC "${CMAKE_SOURCE_DIR}/deps/asio2"
)

add_dependencies(ts3plugin pluginsdk)

set_target_properties(ts3plugin PROPERTIES OUTPUT_NAME "grb_${PLUGIN_VERSION}")

# server
add_executable(server
    src/server/main.cpp
)

target_include_directories(server
    PUBLIC "${CMAKE_SOURCE_DIR}/include"
)
