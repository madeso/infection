int errno;

#include "inf_lua.h"
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "globalGenesis.h"
#include "inventory.h"
#include <errno.h>


lua_State* luaVM=0;

#define LUA_FUNC(x)		int x(lua_State* luaVM)

LUA_FUNC(l_consoleMessage){
	console_message( lua_tostring(luaVM, -1) );
	lua_pushnumber(luaVM, 1);
	return 1;
}

LUA_FUNC(l_gameMessage){
	int args;
	if( (args = lua_gettop(luaVM)) != 1 ){
		for(;args>0; args--)
			lua_tostring(luaVM, -1);
		// todo
		lua_error(luaVM);
		return 0;
	}
	game_message( lua_tostring(luaVM, -1) );
	lua_pushnumber(luaVM, 1);
	return 1;
}

LUA_FUNC(l_errorMessage){
	char str[400];
	int args;
	int i=0;
	char* tstr=0;

	args = lua_gettop(luaVM);
	sprintf(str, "Lua error");
	for(i=0;i<args;i++){
		tstr = lua_tostring(luaVM, -1);
		if( tstr == 0 ){
			continue;
		}
		strcat(str, ": ");
		strcat(str, tstr );
	}
	console_message( str );
	system_message( str );
	lua_pushnumber(luaVM, 1);
	return 1;
}

LUA_FUNC(l_playerHas){
	int res=0;
	res = inventory_has( lua_tostring(luaVM, -1) );
	lua_pushboolean(luaVM, res);
	return 1;
}

LUA_FUNC(l_playerGive){
	inventory_add(lua_tostring(luaVM, -1) );
	lua_pushnumber(luaVM, 1);
	return 1;
}

LUA_FUNC(l_playerLoose){
	inventory_remove(lua_tostring(luaVM, -1) );
	lua_pushnumber(luaVM, 1);
	return 1;
}

LUA_FUNC(lua_panic){
	console_message( "Lua panic" );
	return 0;
}

LUA_FUNC(l_getPos){
	lua_pushnumber(luaVM, XForm.Translation.X);
	lua_pushnumber(luaVM, XForm.Translation.Y);
	lua_pushnumber(luaVM, XForm.Translation.Z);
	return 3;
}

char inf_luaInit(){
	luaVM = lua_open(); // 0 is stack size
	if(! luaVM ) return 0;
	
	luaopen_base	(luaVM);
	luaopen_table	(luaVM);
	luaopen_io		(luaVM);
	luaopen_string	(luaVM);
	luaopen_math	(luaVM);
	luaopen_debug	(luaVM);

	lua_register(luaVM, "print", l_consoleMessage);
	lua_register(luaVM, "print_console", l_consoleMessage);
	lua_register(luaVM, "_ALERT", l_errorMessage);
	lua_register(luaVM, "getPlayerPos", l_getPos);
	lua_register(luaVM, "playerHas", l_playerHas);
	lua_register(luaVM, "playerGive", l_playerGive);
	lua_register(luaVM, "playerLoose", l_playerLoose);
	lua_register(luaVM, "game_message", l_gameMessage);
	lua_atpanic (luaVM, lua_panic);

	/*lua_baselibopen(luaVM);
	lua_iolibopen(luaVM);
	lua_strlibopen(luaVM);
	lua_mathlibopen(luaVM);*/

	return 1;
}

void inf_luaExit(){
	if( luaVM ){
		lua_close(luaVM);
		luaVM = 0;
	}
}

char inf_luafile(char* file){
	char finfile[200];
	sprintf(finfile, ".\\levels\\%s\\LuaScript\\%s", getLevelFileName(), file);
	lua_dofile(luaVM, finfile);
	return 1;
}

char inf_luacommand(char* command){
	lua_dostring(luaVM, command);
	return 1;
}