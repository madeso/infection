/*
playerSounds.h
all the sounds that the player can play.
*/

#ifndef PLAYER_SOUNDS_H
#define PLAYER_SOUNDS_H

#include "sound_system.h"

//some player sounds
soundsys_sound jump[3];
soundsys_sound hurt[7];
soundsys_sound die[5];
soundsys_sound armor[5];

soundsys_sound land_normal[3];
soundsys_sound splash[6];

soundsys_sound breathing_again;
soundsys_sound outofwater;

soundsys_sound boing;

soundsys_sound mugshots[4];

soundsys_sound bob_normal_right[5];
soundsys_sound bob_normal_left[5];

soundsys_sound bob_water_right[5];
soundsys_sound bob_water_left[5];

soundsys_sound bob_ladder_left[5];
soundsys_sound bob_ladder_right[5];
#endif