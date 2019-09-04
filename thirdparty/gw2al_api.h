#pragma once

#define gw2al_hashed_name unsigned long long
#define gw2al_event_id unsigned long

typedef struct gw2al_addon_dsc {
	const wchar_t* name;
	const wchar_t* description;
	unsigned char majorVer;
	unsigned char minorVer;
	unsigned int revision;

	gw2al_addon_dsc* dependList;
} gw2al_addon_dsc;

//api return codes

typedef enum gw2al_api_ret {
	GW2AL_OK,
	GW2AL_FAIL,
	GW2AL_IN_USE,
	GW2AL_NOT_FOUND,
	GW2AL_BAD_DLL,
	GW2AL_DEP_NOT_LOADED,
	GW2AL_DEP_OUTDATED,
	GW2AL_DEP_STILL_LOADED,
	GW2AL_STATIC_LIMIT_HIT
} gw2al_api_ret;

//used to handle events
typedef void(*gw2al_api_event_handler)(void* data);

#define GW2AL_CORE_FUNN_HASH_NAME 1
#define GW2AL_CORE_FUNN_REG_FUN 2
#define GW2AL_CORE_FUNN_UNREG_FUN 3
#define GW2AL_CORE_FUNN_QUERY_FUN 4
#define GW2AL_CORE_FUNN_FILL_VTBL 5
#define GW2AL_CORE_FUNN_UNLOAD_ADDON 6
#define GW2AL_CORE_FUNN_LOAD_ADDON 7
#define GW2AL_CORE_FUNN_QUERY_ADDON 8
#define GW2AL_CORE_FUNN_WATCH_EVENT 9
#define GW2AL_CORE_FUNN_UNWATCH_EVENT 10
#define GW2AL_CORE_FUNN_QUERY_EVENT 11
#define GW2AL_CORE_FUNN_TRIGGER_EVENT 12
#define GW2AL_CORE_FUNN_CLIENT_UNLOAD 13
#define GW2AL_CORE_FUNN_LOG_TEXT 14
#define GW2AL_CORE_FUNN_D3DCREATE_HOOK 15

typedef enum gw2al_log_level {
	LL_INFO = 0,
	LL_ERR,
	LL_WARN,	
	LL_DEBUG
} gw2al_log_level;

typedef struct gw2al_core_vtable {
	//converts string to hash for usage in other functions
	gw2al_hashed_name (*hash_name)(wchar_t* name);

	//register/unregister user functions to be called by other addons
	gw2al_api_ret (*register_function)(void* function, gw2al_hashed_name name);
	void (*unregister_function)(gw2al_hashed_name name);

	//query function pointer from registered list
	void* (*query_function)(gw2al_hashed_name name);

	//fills table of functions using query_function
	void (*fill_vtable)(gw2al_hashed_name* nameList, void** vtable);
			
	//functions to unload/load addons 
	gw2al_api_ret (*unload_addon)(gw2al_hashed_name name);
	gw2al_api_ret (*load_addon)(wchar_t* name);

	//function to get currently loaded addon description
	gw2al_addon_dsc* (*query_addon)(gw2al_hashed_name name);
	
	//simple event api 
	//watch event can add a number of handlers on event name with priority 
	//query event will get internal event id to speedup trigger_event calls

	gw2al_api_ret (*watch_event)(gw2al_event_id id, gw2al_hashed_name subscriber, gw2al_api_event_handler handler, unsigned int priority);
	void (*unwatch_event)(gw2al_event_id id, gw2al_hashed_name subscriber);
	gw2al_event_id (*query_event)(gw2al_hashed_name name);
	unsigned int (*trigger_event)(gw2al_event_id id, void* data);

	//unload function to delete properly unload things on client exit

	void (*client_unload)();

	//simple logging function

	void (*log_text)(gw2al_log_level level, wchar_t* source, wchar_t* text);

} gw2al_core_vtable;

//addon must export this functions as
//gw2addon_get_description
//gw2addon_load
//gw2addon_unload

typedef gw2al_addon_dsc* (*gw2al_addon_get_dsc_proc)();
typedef gw2al_api_ret(*gw2al_addon_load_proc)(gw2al_core_vtable* core_api);
typedef gw2al_api_ret(*gw2al_addon_unload_proc)(int gameExiting);
