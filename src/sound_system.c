#include <windows.h>
#include "sound_system.h"
#include "log.h"
#include "genesis.h"
#include "globalGenesis.h"
#include "inf_message_system.h"

float heroTime = 0.0f;
float otherTime = 0.0f;

//---------------
//sound variables
//---------------

#include "player.h"
#define SETUP_PAN(x)	((options.invertSpeakers!=0)?-1.0f:1.0f) * x

geSound_System	*soundsys =0;
geSound_Def*	sounddefs[SOUNDSYS_NUMBEROFSOUNDS];
geSound*		sounds[SOUNDSYS_NUMBEROFSOUNDS];
soundsys_id LATEST_SOUND = 0;
geBoolean sound_aviable = GE_TRUE;

geSound_Def*	temp_sounddefs[SOUNDSYS_NUMBEROFTEMPORARYS];
geSound*		temp_sounds[SOUNDSYS_NUMBEROFTEMPORARYS];
int				temp_current=0;

geSound_Def*	talk_sounddef=0;
geSound*		talk_sound=0;
geBoolean heroLatest = GE_FALSE;

geSound_Def* load_sound(char *filename)
{
	geVFile *f=0;
	geSound_Def *out=0;

	f = geVFile_OpenNewSystem(NULL, GE_VFILE_TYPE_DOS, filename, NULL, GE_VFILE_OPEN_READONLY);

	if( !f )
	{
		printLog("\t******File: ");
		printLog(filename);
		printLog(" couldn't be loaded\n");
		return 0;
	}

	out = geSound_LoadSoundDef(soundsys, f); 
	geVFile_Close(f);

	return out;
}





geBoolean soundsys_load( HWND hWnd)
{
	soundsys_id id=0;
	sound_aviable = GE_TRUE;
	soundsys = geSound_CreateSoundSystem(hWnd);
	if(! soundsys )
	{
		printLog("Failed to create soundsys.\n");
		sound_aviable = GE_FALSE;
		return GE_TRUE;
	}

	//----------------------------------------------

	for( id=0; id < SOUNDSYS_NUMBEROFSOUNDS; id++)
	{
		sounddefs[id] = 0;
		sounds[id] = 0;
	}

	//-----------------------------------------------

	if(! soundsys_master(1.0f) )
	{
		printLog("Failed to set the master");
		return GE_FALSE;
	}

	return GE_TRUE;
}










void soundsys_free_id(soundsys_id id)
{
	if(! sound_aviable ) return;
	if( id > LATEST_SOUND )
	{
		printLog("\t***sound_system programming error::sound_free_id -  That sound id doesnt exist.\n");
		return;
	}
	geSound_FreeSoundDef(soundsys, sounddefs[id]); 
	sounddefs[id] = 0;
}


void soundsys_destroy()
{
	soundsys_id id=0;
	int i;
    if(! sound_aviable ) return;

	for(id = 0; id < SOUNDSYS_NUMBEROFSOUNDS; id++)
	{
		if( sounddefs[id] ) soundsys_free_id(id);
		else break;
		sounds[id] = 0;
	}

	for(i=0; i <SOUNDSYS_NUMBEROFTEMPORARYS; i++){
		if( temp_sounddefs[i] ){
			geSound_FreeSoundDef( soundsys, temp_sounddefs[i] );
			temp_sounddefs[i] = 0;
		}
	}

	if( talk_sounddef ){
		printLog("Destroying talk sound");
		geSound_FreeSoundDef(soundsys, talk_sounddef);
		talk_sounddef = 0;
		printLog("Done destroying talk sound");
	}

	geSound_DestroySoundSystem(soundsys);
}

void modify( geSound* sound, float toMod ){
	if( sound )
		if( geSound_SoundIsPlaying(soundsys,sound) )
			geSound_ModifySound(soundsys, sound, SOUNDSYS_DEFAULT_VOLUME, SOUNDSYS_DEFAULT_PAN, SOUNDSYS_DEFAULT_FREQUENCY*toMod);
}

void soundsys_updateSound(float ot, float ht){
	soundsys_id id=0;
    if(! sound_aviable ) return;

	heroTime = ht;
	otherTime = ot;
}

geBoolean soundsys_conversation_isTalking(){
	if( !sound_aviable) return GE_FALSE;
	if( talk_sound ){
		return geSound_SoundIsPlaying(soundsys, talk_sound);
	}
	return GE_FALSE;
}

void soundsys_conversation_stopTalkiing(){
	if( !sound_aviable) return;
	if( talk_sound ){
		geSound_StopSound(soundsys, talk_sound );
	}
}

geBoolean soundsys_conversation_heroSay(char* soundFile){
	heroLatest = GE_TRUE;
	if( !sound_aviable) return GE_FALSE;
	if( talk_sounddef ){
		geSound_FreeSoundDef( soundsys, talk_sounddef );
		talk_sounddef = 0;
	}
	talk_sounddef = load_sound(soundFile);
	talk_sound = geSound_PlaySoundDef(soundsys, talk_sounddef, SOUNDSYS_DEFAULT_VOLUME, SOUNDSYS_DEFAULT_PAN, SOUNDSYS_DEFAULT_FREQUENCY *heroTime, GE_FALSE);
	return GE_TRUE;
}

geBoolean soundsys_conversation_personSay(char* soundFile, geVec3d *position, float min, geBoolean ignore){
	geFloat vol = SOUNDSYS_DEFAULT_VOLUME;
	geFloat pan = SOUNDSYS_DEFAULT_PAN;
	geFloat freq = SOUNDSYS_DEFAULT_FREQUENCY;
	heroLatest = GE_FALSE;
	if( !sound_aviable) return GE_FALSE;
	if( talk_sounddef ){
		geSound_FreeSoundDef( soundsys, talk_sounddef );
		talk_sounddef = 0;
	}
	talk_sounddef = load_sound(soundFile);
	if( !talk_sounddef ){
		char error[300];
		sprintf(error, "Failed to load %s", soundFile);
		system_message(error);
		return GE_FALSE;
	}

	if( position ){
		if( ignore ){
			geSound3D_GetConfigIgnoreObstructions(World, &XForm, position, METERS(min), 0.0f, &vol, &pan, &freq); 
		}
		else {
			geSound3D_GetConfig(World, &XForm, position, METERS(min), 0.0f, &vol, &pan, &freq); 
		}
		talk_sound = geSound_PlaySoundDef(soundsys, talk_sounddef, vol, SETUP_PAN(pan), SOUNDSYS_DEFAULT_FREQUENCY *heroTime>1.0f?1.0f:heroTime, GE_FALSE);
	} else {
 		talk_sound = geSound_PlaySoundDef(soundsys, talk_sounddef, SOUNDSYS_DEFAULT_VOLUME, SOUNDSYS_DEFAULT_PAN, SOUNDSYS_DEFAULT_FREQUENCY *heroTime, GE_FALSE);
	}

	if( !talk_sound ) {
		char error[300];
		sprintf(error, "Failed to play sound: %s", soundFile);
		system_message(error);
	}

	return GE_TRUE;
}

void soundsys_conversation_updatePersonSound(geVec3d* position, float min, geBoolean ignore){
	geFloat vol = SOUNDSYS_DEFAULT_VOLUME;
	geFloat pan = SOUNDSYS_DEFAULT_PAN;
	geFloat freq = SOUNDSYS_DEFAULT_FREQUENCY;


	if(! talk_sound ) return;
	if( heroLatest ) return;
	if(! talk_sounddef ) return;
	
	if( !position ){
		return;
	}

	if( ignore )
	{
		geSound3D_GetConfigIgnoreObstructions(World, &XForm, position, METERS(min), 0.0f, &vol, &pan, &freq);
	}
	else
	{
		geSound3D_GetConfig(World, &XForm, position, METERS(min), 0.0f, &vol, &pan, &freq); 
	}

	geSound_ModifySound(soundsys, talk_sound, vol, SETUP_PAN(pan), SOUNDSYS_DEFAULT_FREQUENCY *heroTime>1.0f?1.0f:heroTime);
}

geBoolean soundsys_play_once_sound(char* sound){
	geSound_Def* def = 0;
	if( !sound_aviable) return GE_FALSE;

	def = load_sound(sound);
	if( !def ) return GE_FALSE;

	// check current data and kill if necessarry
	if( temp_sounddefs[temp_current] ){
		// there is data
		if( temp_sounds[temp_current] ){
			if( geSound_SoundIsPlaying(soundsys, temp_sounds[temp_current]) )
			{
				geSound_StopSound(soundsys, temp_sounds[temp_current] );
			}
			temp_sounds[temp_current] = 0;
		}
		geSound_FreeSoundDef( soundsys, temp_sounddefs[temp_current] );
		temp_sounddefs[temp_current] = 0;
	}

	temp_sounds[temp_current] =  geSound_PlaySoundDef(soundsys, def, SOUNDSYS_DEFAULT_VOLUME, SOUNDSYS_DEFAULT_PAN, SOUNDSYS_DEFAULT_FREQUENCY*otherTime, GE_FALSE);
	temp_sounddefs[temp_current] = def;
	
	temp_current++;
	if( temp_current == SOUNDSYS_NUMBEROFTEMPORARYS ) temp_current = 0;
	
	return GE_TRUE;
}

geBoolean soundsys_play_once_3dsound(char* sound, geVec3d *position, float min, geBoolean ignore){
	geFloat vol = SOUNDSYS_DEFAULT_VOLUME;
	geFloat pan = SOUNDSYS_DEFAULT_PAN;
	geFloat freq = SOUNDSYS_DEFAULT_FREQUENCY;
	geSound_Def* def = 0;
	if( !sound_aviable) return GE_FALSE;

	def = load_sound(sound);
	if( !def ) return GE_FALSE;

	// check current data and kill if necessarry
	if( temp_sounddefs[temp_current] ){
		// there is data
		if( temp_sounds[temp_current] ){
			if( geSound_SoundIsPlaying(soundsys, temp_sounds[temp_current]) )
			{
				geSound_StopSound(soundsys, temp_sounds[temp_current] );
			}
			temp_sounds[temp_current] = 0;
		}
		geSound_FreeSoundDef( soundsys, temp_sounddefs[temp_current] );
		temp_sounddefs[temp_current] = 0;
	}

	if( ignore )
	{
		geSound3D_GetConfigIgnoreObstructions(World, &XForm, position, METERS(min), 0.0f, &vol, &pan, &freq); 
	}
	else
	{
		geSound3D_GetConfig(World, &XForm, position, METERS(min), 0.0f, &vol, &pan, &freq); 
	}

	temp_sounds[temp_current] = geSound_PlaySoundDef(soundsys, def, vol, SETUP_PAN(pan), SOUNDSYS_DEFAULT_FREQUENCY*otherTime>1.0f?1.0f:otherTime, GE_FALSE); 

	temp_sounddefs[temp_current] = def;
	
	temp_current++;
	if( temp_current == SOUNDSYS_NUMBEROFTEMPORARYS ) temp_current = 0;
	
	return GE_TRUE;
}


geBoolean soundsys_play3dsound(soundsys_sound *sound, geVec3d *position, float min, geBoolean ignore)
{
	geFloat vol = SOUNDSYS_DEFAULT_VOLUME;
	geFloat pan = SOUNDSYS_DEFAULT_PAN;
	geFloat freq = SOUNDSYS_DEFAULT_FREQUENCY;
	if(! sound_aviable ) return GE_TRUE;

	if( !sound )
		return GE_FALSE;

	if( sound->id >= LATEST_SOUND )
	{
		printLog("\t***sound_system programming error::soundsys_play3dsound -  That sound id doesnt exist.\n");
		return GE_FALSE;
	}
	
	if( ignore )
	{
		geSound3D_GetConfigIgnoreObstructions(World, &XForm, position, METERS(min), 0.0f, &vol, &pan, &freq); 
	}
	else
	{
		geSound3D_GetConfig(World, &XForm, position, METERS(min), 0.0f, &vol, &pan, &freq); 
	}

	soundsys_stop(sound);

	sounds[sound->id] = geSound_PlaySoundDef(soundsys, sounddefs[sound->id], vol, SETUP_PAN(pan), SOUNDSYS_DEFAULT_FREQUENCY*otherTime>1.0f?1.0f:otherTime, GE_FALSE); 
	
	return GE_TRUE;
}

geBoolean soundsys_play_sound(soundsys_sound *sound, geBoolean loop){
	return soundsys_play_sound_ex(sound, loop, TYPE_BOTH);
}

geBoolean soundsys_play_sound_ex(soundsys_sound *sound, geBoolean loop, char type){
	float pan = 0.0f;
	if(! sound_aviable ) return GE_TRUE;
	if( !sound ) return GE_FALSE;

	if( sound->id >= LATEST_SOUND )
	{
		printLog("\t***sound_system programming error::soundsys_play_sound_ex -  That sound id doesnt exist.\n");
		return GE_FALSE;
	}

	if( loop )
	{
		if( sounds[sound->id] )
		{
			if( geSound_SoundIsPlaying(soundsys, sounds[sound->id] ) )
			{
				return GE_TRUE;
			}
		}
	}
	else
	{
		soundsys_stop(sound);
	}

	switch( type ){
	case TYPE_LEFT:
		pan = -0.1f;
		break;
	case TYPE_RIGHT:
		pan = 0.1f;
		break;
	case TYPE_BOTH:
		pan = 0.0f;
	}

	sounds[sound->id] = geSound_PlaySoundDef(soundsys, sounddefs[sound->id], SOUNDSYS_DEFAULT_VOLUME, SETUP_PAN(pan), SOUNDSYS_DEFAULT_FREQUENCY*heroTime, loop); 

	return GE_TRUE;
}

geBoolean soundsys_loadWaw(char* waw, soundsys_sound *sound)
{
	if(! sound_aviable ) return GE_TRUE;
	if( !sound ) return GE_FALSE;

	if( LATEST_SOUND == SOUNDSYS_NUMBEROFSOUNDS )
	{
		printLog("All sounds are used, change the SOUNDSYS_NUMBEROFSOUNDS macro.\n");
		return GE_FALSE;
	}

	sound->id = LATEST_SOUND;
	
	sounddefs[sound->id] = load_sound(waw);
	if(! sounddefs[sound->id] )
	{
		printLog("Failed to load\" ");
		printLog(waw);
		printLog(" \".\n");
		return GE_FALSE;
	}

	LATEST_SOUND++;

	sounds[sound->id] = 0;

	return GE_TRUE;
}

geBoolean soundsys_master(geFloat master)
{
	if(! sound_aviable ) return GE_TRUE;
	return geSound_SetMasterVolume(soundsys, master);
}

void soundsys_stop(soundsys_sound *sound)
{
	if(! sound_aviable ) return;
	if( sound )
	{
		if( sounds[sound->id] )
		{
			if( geSound_SoundIsPlaying(soundsys, sounds[sound->id] ) )
			{
				if( !geSound_StopSound(soundsys, sounds[sound->id] ) )
				{
					printLog("Failed when stopping sound");
				}
			}
		}
	}
}