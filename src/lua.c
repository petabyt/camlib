// Lua interface for camlib. This is not meant to be compiled with camlib.so.
// This is an extension to camlib, and must be compiled with the app.
// You must provide luaptp_get_runtime()
#include <stdio.h>
#include <string.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include <camlib.h>

struct PtpRuntime *luaptp_get_runtime(lua_State *L);

static int mylua_device_info(lua_State *L) {
	struct PtpRuntime *r = luaptp_get_runtime(L);

	if (r->di == NULL) return 1;

	lua_newtable(L);

	lua_pushstring(L, "model");
	lua_pushstring(L, r->di->model);
	lua_settable(L, -3);

	lua_pushstring(L, "propsSupported");
	lua_newtable(L);

	for (int i = 0; i < r->di->props_supported_length; ++i) {
		lua_pushinteger(L, i + 1);
		lua_pushinteger(L, r->di->props_supported[i]);
		lua_settable(L, -3);
	}

	lua_settable(L, -3);

	return 1;
}

static int mylua_take_picture(lua_State *L) {
	struct PtpRuntime *r = luaptp_get_runtime(L);

	if (r->di == NULL) return 1;

	lua_pushinteger(L, 0);

	return 1;
}

static int mylua_send_operation(lua_State *L) {
	struct PtpRuntime *r = luaptp_get_runtime(L);

	int opcode = lua_tointeger(L, 1);

	if (!lua_istable(L, 2)) {
		return luaL_error(L, "Second argument must be an array.");
		return -1;
	}

    int param_length = luaL_len(L, 2);

    // Create an array to store the integers
    int *param_array = (int *)malloc(param_length * sizeof(int));

    for (int i = 1; i <= param_length; ++i) {
        lua_rawgeti(L, 2, i);
        param_array[i - 1] = luaL_checkinteger(L, -1);
        lua_pop(L, 1);
    }

	lua_pushinteger(L, 0);

	return 1;
}

static const luaL_Reg ptplib[] = {
	{"getDeviceInfo",	mylua_device_info},
	{"takePicture",		mylua_take_picture},
	{"sendOperation",	mylua_send_operation},
	{NULL, NULL}
};

LUALIB_API int luaopen_ptp(lua_State *L) {
	luaL_newlib(L, ptplib);
	return 1;
}
