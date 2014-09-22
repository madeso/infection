#include "time_out.h"
#include <string.h>
#include <memory.h>
#include <stdio.h>
#include "console.h"
#include "genesis.h"
#include "inf_lua.h"
#include "init.h" // for level_GetFileName()
#include "conversation.h"

#define TIMOUT_COUNT 10

typedef struct timeout_{
	char script[100];
	float time_left;
	char active;
} timeout;

timeout gto[TIMOUT_COUNT];
int to_index;

void timeout_init(){
	timeout_clear();
	to_index = 0;
}

void timeout_add(float time, const char* script){
	strcpy(gto[to_index].script, script);
	gto[to_index].active = 1;
	gto[to_index].time_left = time;

	to_index++;
	if( to_index >= TIMOUT_COUNT ) to_index = 0;
}

void timeout_iterate(float timePassed){
	int i;
	for(i=0; i<TIMOUT_COUNT; i++){
		if( gto[i].active ){
			gto[i].time_left -= timePassed;
			if( gto[i].time_left <= 0.0f ){
				execute_command(gto[i].script );
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
	}
	to_index=0;
}
