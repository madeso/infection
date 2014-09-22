#include "level.h"
#include "globals.h"


void level_enableLevelFog(){
	if( global_levelHeader ){
		if( global_levelHeader->use_fog ){
			set_fog(
				global_levelHeader->fog_color.r,
				global_levelHeader->fog_color.g,
				global_levelHeader->fog_color.b,
				global_levelHeader->fog_distance, 0);//global_levelHeader->use_clip );
		} else {
			kill_fog();
		}
	} else {
		kill_fog();
	}
}