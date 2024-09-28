#ifndef CAM_LUA_H
#define CAM_LUA_H

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

LUALIB_API int luaopen_ptp(lua_State *L);
lua_State *cam_new_task(int *id);
int lua_script_run_loop(int id);
int cam_run_lua_script(const char *buffer);
int cam_run_lua_script_async(const char *buffer);
const char *cam_lua_get_error();

#endif
