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

#define TYPE_PLAY		0
#define TYPE_STOP		1
#define TYPE_FAIL		2

#define SOUND_SURROUND	-10.0f

								   //----------------
								   // 	3D	|	Freq |
								   // (pan&vol)		 |
								   //----------------
#define TYPE_UNAFFECTED			0  //		|		 |
#define TYPE_2D					1  //		|	X	 |
#define TYPE_3D					2  //	X	|	X	 |
								   //-----------------

geBoolean soundsys_load( HWND hWnd);
void soundsys_destroy();
geBoolean soundsys_play3dsound(soundsys_sound *id, geVec3d *position, float min, geBoolean ignore, char loop);
geBoolean soundsys_play_sound(soundsys_sound *id, geBoolean loop, char typeHandle, float pan, float volume);
geBoolean soundsys_play_sound_ex(soundsys_sound *id, geBoolean loop, char type);

geBoolean soundsys_play_once_sound(char* sound);
geBoolean soundsys_play_once_3dsound(char* sound, geVec3d *position, float min, geBoolean ignore);

geBoolean soundsys_conversation_isTalking();
void soundsys_conversation_stopTalkiing();
geBoolean soundsys_conversation_heroSay(char* soundFile);
geBoolean soundsys_conversation_personSay(char* sound, geVec3d *position, float min, geBoolean ignore);
void soundsys_conversation_updatePersonSound(const geVec3d* position, float min, geBoolean ignore); // deprecated, this is alredy handled by other functions
// see soundsys_update3dAll


geBoolean soundsys_loadSound(char* file, char sample, soundsys_sound *sound, char useChannel, char typePlay);
geBoolean soundsys_master(geFloat master); // sequenced sounds aren't affected...
void soundsys_stop(soundsys_sound *sound);

void soundsys_updateSound(float otherTime);
void soundsys_update3dAll(const geVec3d* listener, geBoolean ignore);

void soundsys_setGameVolume(float vol);
void soundsys_setVolume(soundsys_sound *id, float vol);

geBoolean soundsys_playMusic(char* file);
geBoolean soundsys_changeMusic(char* file); // does not stop the music if file failed to load, and changes the postion to alig with the new song
// so you can have song A with the following chords: EHHGADAEH
// and another song with theese chords:              HGAHDEEDA (the songs must be of the same length)
// it will then keep the position where to old was,      ^ like this
//   the arrow displays the song position, in this example the chord A would become D.
// and note: there is no transition between the sounds, the effect is direct.
void soundsys_stopMusic(); // if playing, else nothing happens

void soundsys_freeTemporaries();

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
#define SOUNDSYS_NUMBEROFSOUNDDEFS		230 // how many that can be stored
#define SOUNDSYS_NUMBEROFSOUNDS			100 // how many sounds that can currently be played
#define SOUNDSYS_NUMBEROFTEMPORARYS		3 // how many tempory sounds that can currently be played
//------------------------------------------------------

#endif // SOUND_SYS_H