// Lua interface for camlib. This is not meant to be compiled with camlib.so.
// This is an extension to camlib, and must be compiled with the app.
// You must provide luaptp_get_runtime()
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include <camlib.h>

struct PtpRuntime *luaptp_get_runtime(lua_State *L);
void lua_json_decode(lua_State *l, const char *json_text, int json_len);
void json_create_config(lua_State *l);

static int mylua_device_info(lua_State *L) {
	struct PtpRuntime *r = luaptp_get_runtime(L);

	if (r->di == NULL) return 1;

	char buffer[2048];
	ptp_device_info_json(r->di, buffer, sizeof(buffer));

	lua_json_decode(L, buffer, strlen(buffer));

	return 1;
}

static int mylua_take_picture(lua_State *L) {
	struct PtpRuntime *r = luaptp_get_runtime(L);

	if (r->di == NULL) return 1;

	int rc = ptp_pre_take_picture(r);
	if (rc) goto err;

	rc = ptp_take_picture(r);
	if (rc) goto err;

	err:;
	lua_pushinteger(L, rc);

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

	struct PtpCommand cmd;
	cmd.code = opcode;
	cmd.param_length = param_length;

	// Create an array to store the integers
	int *param_array = (int *)malloc(param_length * sizeof(int));

	for (int i = 1; i <= param_length; ++i) {
		lua_rawgeti(L, 2, i);
		cmd.params[i - 1] = luaL_checkinteger(L, -1);
		lua_pop(L, 1);
	}

	lua_pushinteger(L, 0);

	int rc = ptp_generic_send(r, &cmd);

	//lua_pushinteger(L, rc);

	printf("length: %d\n", ptp_get_payload_length(r));

	lua_newtable(L);
	for (int i = 0; i < ptp_get_payload_length(r); ++i) {
		lua_pushinteger(L, i + 1);
		lua_pushinteger(L, (int)(r->data[i]));
		lua_settable(L, -3);
	}

	return 1;
}

static int mylua_test(lua_State *L) {
	struct PtpRuntime *r = luaptp_get_runtime(L);

	char *t = "{\"hello\": 123}";

	//json_create_config(L);
	lua_json_decode(L, t, strlen(t));

	return 1;
}

static const luaL_Reg ptplib[] = {
	{"getDeviceInfo",	mylua_device_info},
	{"takePicture",		mylua_take_picture},
	{"test",			mylua_test},
	{"sendOperation",	mylua_send_operation},
	{NULL, NULL}
};

static void new_const(lua_State *L, char *name, int val) {
	lua_pushstring(L, name);
	lua_pushnumber(L, val);
	lua_settable(L, -3);
}

LUALIB_API int luaopen_ptp(lua_State *L) {
	luaL_newlib(L, ptplib);

	new_const(L, "OK", 0);
	new_const(L, "NO_DEVICE", -1);
	new_const(L, "NO_PERM", -2);
	new_const(L, "OPEN_FAIL", -3);
	new_const(L, "OUT_OF_MEM", -4);
	new_const(L, "IO_ERR", -5);
	new_const(L, "RUNTIME_ERR", -6);
	new_const(L, "UNSUPPORTED", -7);
	new_const(L, "CHECK_CODE", -8);

	return 1;
}