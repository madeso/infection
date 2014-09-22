int errno;

#include "inf_lua.h"
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "globalGenesis.h"
#include "inventory.h"
#include <errno.h>
#include "inf_message_system.h"
#include "console.h"
#include "init.h" // for getLevelFileName()

//lua_State* luaVM=0;
//#define LUA_FUNC(x)		int x(lua_State* luaVM)

LUA_FUNC(consoleMessage){
	console_message( lua_tostring(luaVM, -1) );
	lua_pushnumber(luaVM, 1);
	return 1;
}

LUA_FUNC(gameMessage){
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

LUA_FUNC(errorMessage){
	char str[400];
	int args;
	int i=0;
	const char* tstr=0;

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

LUA_FUNC(playerHas){
	int res=0;
	res = inventory_has( lua_tostring(luaVM, -1) );
	lua_pushboolean(luaVM, res);
	return 1;
}

LUA_FUNC(playerGive){
	inventory_add(lua_tostring(luaVM, -1) );
	lua_pushnumber(luaVM, 1);
	return 1;
}

LUA_FUNC(playerLoose){
	inventory_remove(lua_tostring(luaVM, -1) );
	lua_pushnumber(luaVM, 1);
	return 1;
}

LUA_FUNC(lua_panic){
	console_message( "Lua panic" );
	exit_application();
	return 0;
}

LUA_FUNC(getPos){
	lua_pushnumber(luaVM, XForm.Translation.X);
	lua_pushnumber(luaVM, XForm.Translation.Y);
	lua_pushnumber(luaVM, XForm.Translation.Z);
	return 3;
}

char inf_luaInit(){
	if( luaVM ) {
		// hmm
		return 0;
	}
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
	lua_atpanic (luaVM, l_lua_panic);

	/*lua_baselibopen(luaVM);
	lua_iolibopen(luaVM);
	lua_strlibopen(luaVM);
	lua_mathlibopen(luaVM);*/

	return 1;
}

void inf_luaExit(){
	if( luaVM ){
		lua_close(luaVM);
		//luaVM = 0;
	}
	luaVM = 0;
}

char inf_do_luafile(char* file){
	lua_dofile(luaVM, file);
	return 1;
}

char inf_luafile(char* file){
	char finfile[400];
	sprintf(finfile, ".\\levels\\%s\\LuaScript\\%s", getLevelFileName(), file);
	lua_dofile(luaVM, finfile);
	return 1;
}

char inf_luacommand(const char* command){
	if( luaVM ) {
		lua_dostring(luaVM, command);
		return 1;
	}
	//printLog("Lua VM is null in inf_luacommand(const char*)! why?\n\n");
	return 1;
}