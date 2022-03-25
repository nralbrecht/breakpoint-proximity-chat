cmake_minimum_required (VERSION 3.17)

set(PLUGIN_VERSION "0.1.8")

project(GRBTS3Plugin
    VERSION "${PLUGIN_VERSION}"
    DESCRIPTION "Tom Clancy's Ghost Recon: Breakpoint"
    LANGUAGES "CXX"
)

set(CMAKE_CXX_STANDARD 20)
# set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(ExternalProject)

set(JSON_BuildTests OFF CACHE INTERNAL "")
add_subdirectory(deps/json)

ExternalProject_Add(pluginsdk
    URL http://dl.4players.de/ts/client/pluginsdk/pluginsdk_3.1.1.1.zip
    PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/deps/pluginsdk
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
)

add_subdirectory(${CMAKE_SOURCE_DIR}/deps/sqlite3)

# plugin
add_library(ts3plugin
    SHARED
    ${CMAKE_SOURCE_DIR}/src/plugin/main.cpp
    ${CMAKE_SOURCE_DIR}/src/plugin/PluginManager.cpp
    ${CMAKE_SOURCE_DIR}/src/plugin/NetworkManager.cpp
    ${CMAKE_SOURCE_DIR}/src/plugin/Logger.cpp
    ${CMAKE_SOURCE_DIR}/src/plugin/ClientStateManager.cpp
    ${CMAKE_SOURCE_DIR}/src/plugin/PluginConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/plugin/RadioAudioProcessor.cpp
    ${CMAKE_SOURCE_DIR}/src/plugin/Vector3f.cpp
    ${CMAKE_SOURCE_DIR}/src/plugin/game/GameHandler.cpp
    ${CMAKE_SOURCE_DIR}/src/plugin/game/ProcessWrapper.cpp
    ${CMAKE_SOURCE_DIR}/src/plugin/game/ProcessWrapper.cpp
    ${CMAKE_SOURCE_DIR}/src/plugin/game/GRB.cpp
    ${CMAKE_SOURCE_DIR}/src/plugin/game/GRBVulkan.cpp
    ${CMAKE_SOURCE_DIR}/deps/DSPFilters/shared/DSPFilters/source/Bessel.cpp
    ${CMAKE_SOURCE_DIR}/deps/DSPFilters/shared/DSPFilters/source/Biquad.cpp
    ${CMAKE_SOURCE_DIR}/deps/DSPFilters/shared/DSPFilters/source/Butterworth.cpp
    ${CMAKE_SOURCE_DIR}/deps/DSPFilters/shared/DSPFilters/source/Cascade.cpp
    ${CMAKE_SOURCE_DIR}/deps/DSPFilters/shared/DSPFilters/source/ChebyshevI.cpp
    ${CMAKE_SOURCE_DIR}/deps/DSPFilters/shared/DSPFilters/source/ChebyshevII.cpp
    ${CMAKE_SOURCE_DIR}/deps/DSPFilters/shared/DSPFilters/source/Custom.cpp
    ${CMAKE_SOURCE_DIR}/deps/DSPFilters/shared/DSPFilters/source/Design.cpp
    ${CMAKE_SOURCE_DIR}/deps/DSPFilters/shared/DSPFilters/source/Documentation.cpp
    ${CMAKE_SOURCE_DIR}/deps/DSPFilters/shared/DSPFilters/source/Elliptic.cpp
    ${CMAKE_SOURCE_DIR}/deps/DSPFilters/shared/DSPFilters/source/Filter.cpp
    ${CMAKE_SOURCE_DIR}/deps/DSPFilters/shared/DSPFilters/source/Legendre.cpp
    ${CMAKE_SOURCE_DIR}/deps/DSPFilters/shared/DSPFilters/source/Param.cpp
    ${CMAKE_SOURCE_DIR}/deps/DSPFilters/shared/DSPFilters/source/PoleFilter.cpp
    ${CMAKE_SOURCE_DIR}/deps/DSPFilters/shared/DSPFilters/source/RBJ.cpp
    ${CMAKE_SOURCE_DIR}/deps/DSPFilters/shared/DSPFilters/source/RootFinder.cpp
    ${CMAKE_SOURCE_DIR}/deps/DSPFilters/shared/DSPFilters/source/State.cpp
)

target_include_directories(ts3plugin
    PUBLIC "${CMAKE_SOURCE_DIR}/include"
	PUBLIC "${CMAKE_SOURCE_DIR}/deps/pluginsdk/src/pluginsdk/include"
    PUBLIC "${CMAKE_SOURCE_DIR}/deps/asio2"
    PUBLIC "${CMAKE_SOURCE_DIR}/deps/DSPFilters/shared/DSPFilters/include"
)

add_dependencies(ts3plugin pluginsdk)
target_link_libraries(ts3plugin PUBLIC nlohmann_json::nlohmann_json)

set_target_properties(ts3plugin PROPERTIES OUTPUT_NAME "grb_${PLUGIN_VERSION}")

# server
add_executable(server
    ${CMAKE_SOURCE_DIR}/src/server/main.cpp
    ${CMAKE_SOURCE_DIR}/src/server/StateManager.cpp
    ${CMAKE_SOURCE_DIR}/src/server/PositionLogger.cpp
)

target_include_directories(server
    PUBLIC "${CMAKE_SOURCE_DIR}/include"
    PUBLIC "${CMAKE_SOURCE_DIR}/deps/asio2"
    PUBLIC "${CMAKE_SOURCE_DIR}/deps/sqlite3/src"
)

target_link_libraries(server PUBLIC nlohmann_json::nlohmann_json)
target_link_libraries(server PUBLIC sqlite3)

# test_tool
add_executable(test_tool
    src/test_tool/main.cpp
)

target_include_directories(test_tool
    PUBLIC "${CMAKE_SOURCE_DIR}/deps/pluginsdk/src/pluginsdk/include"
)
