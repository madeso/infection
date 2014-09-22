/*
sound_system.h
a basic wrapper for the genesis soundsystem. Every sound that has been loaded is
also cleaned by this simple wrapper.
*/

#include "genesis.h"
#include "windows.h"

#ifndef SOUND_SYSTEM_H
#define SOUND_SYSTEM_H

//-------------------------------------------------------
//typedefs
//-------------------------------------------------------

typedef unsigned char soundsys_id;

typedef struct soundsys_sound_tag
{
	soundsys_id id;
}soundsys_sound;

//-------------------------------------------------------
//functions
//-------------------------------------------------------

#define TYPE_LEFT	0
#define TYPE_RIGHT	1
#define TYPE_BOTH	2

geBoolean soundsys_load( HWND hWnd);
void soundsys_destroy();
geBoolean soundsys_play3dsound(soundsys_sound *sound, geVec3d *position, float min, geBoolean ignore);
geBoolean soundsys_play_sound(soundsys_sound *id, geBoolean loop);
geBoolean soundsys_play_sound_ex(soundsys_sound *id, geBoolean loop, char type);

geBoolean soundsys_play_once_sound(char* sound);
geBoolean soundsys_play_once_3dsound(char* sound, geVec3d *position, float min, geBoolean ignore);

geBoolean soundsys_conversation_isTalking();
void soundsys_conversation_stopTalkiing();
geBoolean soundsys_conversation_heroSay(char* soundFile);
geBoolean soundsys_conversation_personSay(char* sound, geVec3d *position, float min, geBoolean ignore);
void soundsys_conversation_updatePersonSound(geVec3d* position, float min, geBoolean ignore);


geBoolean soundsys_loadWaw(char* waw, soundsys_sound *sound);
geBoolean soundsys_master(geFloat master);
void soundsys_stop(soundsys_sound *sound);

void soundsys_updateSound(float otherTime, float heroTime);

//-------------------------------------------------------
//Default setups
//-------------------------------------------------------

#define SOUNDSYS_DEFAULT_VOLUME					1.0f
#define SOUNDSYS_DEFAULT_PAN					0.0f
#define SOUNDSYS_DEFAULT_FREQUENCY				1.0f

//-------------------------------------------------------
//latest sound
//-------------------------------------------------------

//------------------------------------------------------
#define SOUNDSYS_NUMBEROFSOUNDS			150
#define SOUNDSYS_NUMBEROFTEMPORARYS		3
//------------------------------------------------------

#endif // SOUND_SYS_H