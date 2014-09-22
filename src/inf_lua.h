#ifndef __INF_LUA_H
#define __INF_LUA_H

char inf_luaInit();
void inf_luaExit();
char inf_luafile(char* file);
char inf_do_luafile(char* file);
char inf_luacommand(const char* command);

#include "lua.h"
lua_State* luaVM;

#define LUA_FUNC(x)		int l_##x(lua_State* luaVM)
#define LOAD_FUNC(x)	lua_register(luaVM, #x, l_##x)

#endif