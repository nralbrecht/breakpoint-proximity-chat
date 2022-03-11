#define _CRT_SECURE_NO_WARNINGS

#include <memory>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "teamspeak/public_errors.h"
#include "teamspeak/public_errors_rare.h"
#include "teamspeak/public_definitions.h"
#include "teamspeak/public_rare_definitions.h"
#include "teamspeak/clientlib_publicdefinitions.h"
#include "ts3_functions.h"

#include "main.h"
#include "PluginManager.h"


#define _strcpy(dest, destSize, src) { strncpy(dest, src, destSize - 1); (dest)[destSize - 1] = '\0'; }

#define PLUGIN_API_VERSION 25

#define PATH_BUFSIZE 512
#define COMMAND_BUFSIZE 128
#define INFODATA_BUFSIZE 128
#define SERVERINFO_BUFSIZE 256
#define CHANNELINFO_BUFSIZE 512
#define RETURNCODE_BUFSIZE 128

static char* pluginID = NULL;
static struct TS3Functions ts3Functions;
std::unique_ptr<PluginManager> pluginManager = NULL;

/*********************************** Required functions ************************************/

const char* ts3plugin_name() {
	return "Tom Clancy's Ghost Recon Breakpoint";
}

const char* ts3plugin_version() {
    return "0.1.0";
}

int ts3plugin_apiVersion() {
	return PLUGIN_API_VERSION;
}

const char* ts3plugin_author() {
    return "superananas";
}

const char* ts3plugin_description() {
    return "Provides 3D audio and radio communication for Tom Clancy's Ghost Recon Breakpoint.";
}

void ts3plugin_setFunctionPointers(const struct TS3Functions funcs) {
    ts3Functions = funcs;
}

int ts3plugin_init() {
	try {
		pluginManager = std::make_unique<PluginManager>(ts3Functions);
    	return 0;
	}
	catch(const std::exception& e) {
		// Error intializing plugin manager. Return 1 to indicate error.
		ts3Functions.logMessage(e.what(), LogLevel_ERROR, "GRB", ts3Functions.getCurrentServerConnectionHandlerID());
		return 1;
	}
}

void ts3plugin_shutdown() {
	pluginManager.reset();

	/* Free pluginID if we registered it */
	if(pluginID) {
		free(pluginID);
		pluginID = NULL;
	}
}

/****************************** Optional functions ********************************/
/*
 * If the plugin wants to use error return codes, plugin commands, hotkeys or menu items, it needs to register a command ID. This function will be
 * automatically called after the plugin was initialized. This function is optional. If you don't use these features, this function can be omitted.
 * Note the passed pluginID parameter is no longer valid after calling this function, so you must copy it and store it in the plugin.
 */
void ts3plugin_registerPluginID(const char* id) {
	const size_t sz = strlen(id) + 1;
	pluginID = (char*)malloc(sz * sizeof(char));
	_strcpy(pluginID, sz, id);  /* The id buffer will invalidate after exiting this function */
}

/*
 * Implement the following three functions when the plugin should display a line in the server/channel/client info.
 * If any of ts3plugin_infoTitle, ts3plugin_infoData or ts3plugin_freeMemory is missing, the info text will not be displayed.
 */

/* Static title shown in the left column in the info frame */
const char* ts3plugin_infoTitle() {
	return "GRB info";
}

/*
 * Dynamic content shown in the right column in the info frame. Memory for the data string needs to be allocated in this
 * function. The client will call ts3plugin_freeMemory once done with the string to release the allocated memory again.
 * Check the parameter "type" if you want to implement this feature only for specific item types. Set the parameter
 * "data" to NULL to have the client ignore the info data.
 */
void ts3plugin_infoData(uint64 serverConnectionHandlerID, uint64 id, enum PluginItemType type, char** data) {
	char* uuid;

	if (type == PLUGIN_CLIENT) {
		if(ts3Functions.getClientVariableAsString(serverConnectionHandlerID, (anyID)id, CLIENT_UNIQUE_IDENTIFIER, &uuid) != ERROR_ok) {
			printf("Error getting client nickname\n");
			return;
		}

		*data = (char*)malloc(INFODATA_BUFSIZE * sizeof(char));  /* Must be allocated in the plugin! */
		snprintf(*data, INFODATA_BUFSIZE, "The UUID is [I]\"%s\"[/I]", uuid);  /* bbCode is supported. HTML is not supported */
		ts3Functions.freeMemory(uuid);
	}
	else {
		data = NULL;
	}
}

void ts3plugin_freeMemory(void* data) {
	free(data);
}

/* Helper function to create a hotkey */
static struct PluginHotkey* createHotkey(const char* keyword, const char* description) {
	struct PluginHotkey* hotkey = (struct PluginHotkey*)malloc(sizeof(struct PluginHotkey));
	_strcpy(hotkey->keyword, PLUGIN_HOTKEY_BUFSZ, keyword);
	_strcpy(hotkey->description, PLUGIN_HOTKEY_BUFSZ, description);
	return hotkey;
}

/* Some makros to make the code to create hotkeys a bit more readable */
#define BEGIN_CREATE_HOTKEYS(x) const size_t sz = x + 1; size_t n = 0; *hotkeys = (struct PluginHotkey**)malloc(sizeof(struct PluginHotkey*) * sz);
#define CREATE_HOTKEY(a, b) (*hotkeys)[n++] = createHotkey(a, b);
#define END_CREATE_HOTKEYS (*hotkeys)[n++] = NULL; assert(n == sz);

void ts3plugin_initHotkeys(struct PluginHotkey*** hotkeys) {
	BEGIN_CREATE_HOTKEYS(2);  /* Create 3 hotkeys. Size must be correct for allocating memory. */
	CREATE_HOTKEY("radio_activate", "Radio start transmitting");
	CREATE_HOTKEY("radio_deactivate", "Radio stop transmitting");
	END_CREATE_HOTKEYS;
}

/************************** TeamSpeak callbacks ***************************/
void ts3plugin_onHotkeyEvent(const char* keyword) {
	if (pluginManager) {
		return;
	}

	if (strncmp(keyword, "radio_activate", 15) == 0) {
		pluginManager->radioActivate(true);
	}
	else if (strncmp(keyword, "radio_deactivate", 17) == 0){
		pluginManager->radioActivate(false);
	}
}
