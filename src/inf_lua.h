#ifndef __INF_LUA_H
#define __INF_LUA_H

char inf_luaInit();
void inf_luaExit();
char inf_luafile(char* file);
char inf_luacommand(char* command);

#endif