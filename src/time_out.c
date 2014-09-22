#include "time_out.h"
#include <string.h>
#include <memory.h>
#include <stdio.h>
#include "console.h"
#include "genesis.h"
#include "inf_lua.h"


#define TIMOUT_COUNT 10

typedef struct timeout_{
	char script[100];
	float time_left;
	char active;
	char type;
} timeout;

timeout gto[TIMOUT_COUNT];
int to_index;

void timeout_init(){
	timeout_clear();
	to_index = 0;
}

void timeout_add(float time, char* script, char type){
	strcpy(gto[to_index].script, script);
	gto[to_index].active = 1;
	gto[to_index].time_left = time;
	gto[to_index].type = type;

	to_index++;
	if( to_index >= TIMOUT_COUNT ) to_index = 0;
}

void timeout_iterate(float timePassed){
	int i;
	for(i=0; i<TIMOUT_COUNT; i++){
		if( gto[i].active ){
			gto[i].time_left -= timePassed;
			if( gto[i].time_left <= 0.0f ){
				switch(gto[i].type){
				case 0:
					execute_scriptFile(gto[i].script);
					break;
				case 1:
					startConversation(getLevelFileName(), gto[i].script, GE_FALSE);
					break;
				case 2:
					if(! inf_luafile( gto[i].script ) ){
						char str[200];
						sprintf(str, "Failed to execute lua file: %s", gto[i].script );
						console_message(str);
					}
					break;

				case 3:
					if(! inf_luacommand( gto[i].script ) ){
						char str[200];
						sprintf(str, "Failed to execute lua command: %s", gto[i].script );
						console_message(str);
					}
					break;
				case 4:
				default:
					execute_command(gto[i].script );
					break;
				}
				gto[i].active = 0;
			}
		}
	}
}

void timeout_clear(){
	int i;
	for(i=0; i<TIMOUT_COUNT; i++){
		gto[i].active = 0;
		gto[i].time_left = 0.0f;
		memset(gto[i].script, 0, sizeof(char)*100);
		gto[i].type = 0;
	}
	to_index=0;
}
