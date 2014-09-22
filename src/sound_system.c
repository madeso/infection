#include <windows.h>
#include "sound_system.h"
#include "fmod.h"
#include <string.h>

#include "log.h"
#include "genesis.h"
#include "globalGenesis.h"
#include "inf_message_system.h"
#include "console.h"
#include "player.h"

#define SETUP_PAN(x)	((options.invertSpeakers!=0)?-1.0f:1.0f) * x

//---------------
//sound variables
//---------------
#define TYPE_PCM		0
#define TYPE_SEQ		1
#define TYPE_STREAM		2

typedef struct ssound_ {
	char used; // is this sound-entry loaded?
	void* handle; // the definition of the sound
	char type; // the type, pcm, sequenced or streamed
	char useChannel; // use the channel or simply "let the sound go"
	int channel; // the channel to the sound that is beeing played, it WILL contain the latest played channel integer no matter what useChannel says.
	char modType; // how to change this sound
	const char* file; // original filename - only pointer - no actual data is stored!

	// 3d if modType permits it
	geVec3d* position;
	float min;
	geBoolean ignore;
} SSound;

void ssound_clear(SSound* sound){
	sound->used = 0;
	sound->handle = 0;
	sound->type = 0;
	sound->useChannel = 0;
	sound->channel = 0;
	sound->modType = 0;
	sound->file = 0;
	sound->position = 0;
	sound->min = 0.0f;
	sound->ignore = GE_FALSE;
}

typedef struct schangeable_{
	char used;
	int channel;

	geVec3d position;
	float min;
	geBoolean ignore;
	geBoolean use3d;
} SChangeable;

void schangeable_clear(SChangeable* sound){
	sound->used = 0;
	sound->channel = 0;

	geVec3d_Clear(&(sound->position));
	sound->min = 0.0f;
	sound->ignore = GE_FALSE;
	sound->use3d = GE_FALSE;
}

SSound soundSystem_sounds[SOUNDSYS_NUMBEROFSOUNDDEFS];
SSound soundSystem_temp[SOUNDSYS_NUMBEROFTEMPORARYS];
SChangeable soundSystem_change[SOUNDSYS_NUMBEROFSOUNDS];
SSound playerSoundDef;
SSound songDef;

unsigned int soundSys_nextIndex=0;
unsigned int sound_nextIndex=0;
unsigned int sound_tempIndex=0;
float volume = 1.0f; // changed by sound_modder, no other code should change this var!
float musicVolume = 1.0f;

void soundsys_logError(char* reason){
	console_message(reason);
	printLog(reason); printLog("\n");
	error(reason);
}

geBoolean failedToLoadSound(char* file){
	char str[400];
	sprintf(str, "Soundsys: Failed to load %s", file);
	soundsys_logError(str);
	return GE_FALSE;
}

char isSequencedBased(char* fileName){
	//char ext[10];
	char* ext = 0;
	//int index;

	ext = strrchr( fileName, '.' );
	if( ext == 0 ) return 0;
	ext += 1;
	
	if( stricmp(ext, "wav") == 0 ) return 0;
	if( stricmp(ext, "mp3") == 0 ) return 0;
	if( stricmp(ext, "ogg") == 0 ) return 0;
	if( stricmp(ext, "raw") == 0 ) return 0;
	if( stricmp(ext, "mp2") == 0 ) return 0;

	if( stricmp(ext, "mod") == 0 ) return 1;
	if( stricmp(ext, "s3m") == 0 ) return 1;
	if( stricmp(ext, "xm") == 0 ) return 1;
	if( stricmp(ext, "it") == 0 ) return 1;
	if( stricmp(ext, "mid") == 0 ) return 1;
	if( stricmp(ext, "rmi") == 0 ) return 1;
	if( stricmp(ext, "sgt") == 0 ) return 1;
	if( stricmp(ext, "fsb") == 0 ) return 1;

	return 0;
}

void sound_setPaused(SSound* sound, char paused){
	if(! sound->used ) return;
	if(! sound->useChannel ) return;
	if( sound->channel < 0 ) return;
	switch(sound->type){
	case TYPE_PCM:
		FSOUND_SetPaused(sound->channel, ((paused)?TRUE:FALSE) );
		break;
	case TYPE_SEQ:
		FMUSIC_SetPaused((FMUSIC_MODULE*)(sound->handle), ((paused)?TRUE:FALSE) );
		break;
	case TYPE_STREAM:
		FSOUND_SetPaused(sound->channel, ((paused)?TRUE:FALSE) );
		break;
	default:
		// @@@todo@@@: log this
		return;
	}
}

void sound_setFrequency(SSound* sound, int freq){
	if(! sound->used ) return;
	if(! sound->useChannel ) return;
	if( sound->channel < 0 ) return;
	switch(sound->type){
	case TYPE_PCM:
		FSOUND_SetFrequency(sound->channel, freq);
		break;
	case TYPE_SEQ:
		//unsupported
		break;
	case TYPE_STREAM:
		FSOUND_SetFrequency(sound->channel, freq);
		break;
	default:
		// @@@todo@@@: log this
		return;
	}
}


void setFreqChannelAll(int freq){
	int i;
	for(i=0; i<SOUNDSYS_NUMBEROFSOUNDS; i++){
		if( soundSystem_change[i].channel < 0 ) continue;
		FSOUND_SetFrequency(soundSystem_change[i].channel, freq);
	}
}

void setFreqAll(float theFreq){
	int i;
	int freq;
	freq = (int) (44100 * theFreq);
	if (freq<100) freq = 100;
	if (freq>705600) freq = 705600;
	for(i=0; i<SOUNDSYS_NUMBEROFSOUNDDEFS; i++){
		if( soundSystem_sounds[i].channel < 0 ) continue;
		if( soundSystem_sounds[i].modType == TYPE_UNAFFECTED ) continue;
		if(! soundSystem_sounds[i].useChannel ) continue;
		FSOUND_SetFrequency(soundSystem_sounds[i].channel, freq);
	}
	if( playerSoundDef.used && playerSoundDef.channel >= 0 ){
		FSOUND_SetFrequency(playerSoundDef.channel, freq);
	}
	setFreqChannelAll(freq);
}

void setVolumeChannelAll(int vol){
	int i;
	for(i=0; i<SOUNDSYS_NUMBEROFSOUNDS; i++){
		if( soundSystem_change[i].channel < 0 ) continue;
		if( !soundSystem_change[i].use3d ) continue; // 3d is handled differently
		FSOUND_SetVolume(soundSystem_change[i].channel, vol);
	}
}
void setVolumeAll(){
	int i;
	int vol = (int)(255*volume);
	for(i=0; i<SOUNDSYS_NUMBEROFSOUNDDEFS; i++){
		if( soundSystem_sounds[i].channel < 0 ) continue;
		if( soundSystem_sounds[i].modType != TYPE_2D ) continue;
		if(! soundSystem_sounds[i].useChannel ) continue;
		FSOUND_SetVolume(soundSystem_sounds[i].channel,vol);
	}
	if( playerSoundDef.used && playerSoundDef.channel >= 0 ){
		FSOUND_SetVolume(playerSoundDef.channel,vol);
	}
	setVolumeChannelAll(vol);
}

void soundsys_setVolume(soundsys_sound *id, float volume){
	int vol = (int)(255*volume);
	/*
	char temp[200];
	sprintf(temp, "Vol is %f => %i", volume, vol);
	console_message(temp);*/
	
	if( soundSystem_sounds[id->id].channel < 0 ) return;
	if( soundSystem_sounds[id->id].modType != TYPE_UNAFFECTED ) return;
	if(! soundSystem_sounds[id->id].useChannel ) return;
	FSOUND_SetVolume(soundSystem_sounds[id->id].channel, vol);
}

char set3dsound(int channel, const geVec3d* listener, geVec3d* pos, geBoolean ignore, float min){
	static float thePan = 0.0f;
	static float theVol = 1.0f;
	static float theFreq = 1.0f;
	static int vol = 0;
	static int pan = 0;
	
	if( !listener ) return 0;
	if( !pos ) return 0;
	if( channel < 0 ) return 0;

	if(! FSOUND_IsPlaying(channel) == TRUE ){
		return 0;
	}

	// gather values
	if( ignore )
	{
		geSound3D_GetConfigIgnoreObstructions(World, &XForm, pos, METERS(min), 0.0f, &theVol, &thePan, &theFreq); 
	}
	else
	{
		geSound3D_GetConfig(World, &XForm, pos, METERS(min), 0.0f, &theVol, &thePan, &theFreq); 
	}

	// convert values
	vol = (int)(255*theVol * volume); // set to game volume :)
	pan = (int) (255*((SETUP_PAN(thePan)+1.0f)/2.0f));

	// set channel
	FSOUND_SetPan(channel,pan);
	FSOUND_SetVolume(channel,vol);

	return 1;
}

void update3dChannelAll(const geVec3d* listener, geBoolean ignore){
	int i;
	for(i=0; i<SOUNDSYS_NUMBEROFSOUNDS; i++){
		if( soundSystem_change[i].use3d ){
			if(! soundSystem_change[i].used ) continue;
			//if(! soundSystem_change[i].position ) continue; // only 3d sounds have a position
			if( soundSystem_change[i].channel < 0 ) continue;
			if(! set3dsound(soundSystem_change[i].channel, listener, &(soundSystem_change[i].position), ignore, soundSystem_change[i].min) )
				soundSystem_change[i].channel = -1; // optimized if this is playing then don't  stop the channel
		}
	}
}

void soundsys_update3dAll(const geVec3d* listener, geBoolean ignore){
	int i;
	for(i=0; i<SOUNDSYS_NUMBEROFSOUNDDEFS; i++){
		if(! soundSystem_sounds[i].used ) continue;
		if( soundSystem_sounds[i].modType != TYPE_3D ) continue;
		if( soundSystem_sounds[i].channel < 0 ) continue;
		if(! soundSystem_sounds[i].useChannel ) continue;
		if(! soundSystem_sounds[i].position ) continue;
		if(! set3dsound(soundSystem_sounds[i].channel, listener, soundSystem_sounds[i].position, ignore, soundSystem_sounds[i].min) )
			soundSystem_sounds[i].channel = -1; // optimize if sound has stopped :)
	}
	if( playerSoundDef.used && playerSoundDef.channel >= 0 ){
		if( playerSoundDef.position )
			if(! set3dsound(playerSoundDef.channel, listener, playerSoundDef.position, ignore, playerSoundDef.min) )
				playerSoundDef.channel = -1;
	}
	update3dChannelAll(listener, ignore);
}

void sound_setPanVol(SSound* sound, float thePan, float theVol){
	int vol;
	int pan = -10;
	if(! sound->used ) return;
	//if(! sound->useChannel ) return;
	if( sound->channel < 0 ) return;
	if( pan >= -1.0f && pan <= 1.0f )
		pan = (int) (255*((SETUP_PAN(thePan)+1.0f)/2.0f));

	// keep the game volume :)
	if( sound->modType == TYPE_2D ){
		 vol = (int)(255*theVol * volume);
	} else {
		 vol = (int)(255*theVol);
	}
	
	switch(sound->type){
	case TYPE_PCM:
	case TYPE_STREAM:
		if( pan ==-10 ){
			// assume surround
			FSOUND_SetSurround(sound->channel,TRUE);
		}
		else {
			// use pan
			FSOUND_SetPan(sound->channel,pan);
		}
		FSOUND_SetVolume(sound->channel,vol);
		break;
	case TYPE_SEQ:
		//unsupported
		break;
	default:
		// @@@todo@@@: log this
		return;
	}
}

char sound_isPlaying(SSound* sound){
	if(! sound->used ) return 0;
	if(! sound->useChannel ) return 0;
	if( sound->channel < 0 ) return 0;
	switch(sound->type){
	case TYPE_PCM:
		if( FSOUND_IsPlaying(sound->channel) == TRUE ){
			return 1;
		}
		return 0;
	case TYPE_SEQ:
		if( FMUSIC_IsPlaying((FMUSIC_MODULE*)(sound->handle)) == TRUE ){
			return 1;
		}
		return 0;
	case TYPE_STREAM:
		// ????????????????????????
		if( FSOUND_IsPlaying(sound->channel) == TRUE ){
			return 1;
		}
		return 0;
	default:
		// @@@todo@@@: log this
		return 0;
	}
	return 1;
}

void sound_stop(SSound* sound){
	int res;
	if(! sound->used ) return;
	if(! sound->useChannel ) return;
	if( sound->channel < 0 ) return;
	switch(sound->type){
	case TYPE_PCM:
		FSOUND_StopSound(sound->channel);
		break;
	case TYPE_SEQ:
		FMUSIC_StopSong((FMUSIC_MODULE*)(sound->handle));
		break;
	case TYPE_STREAM:
		res = FSOUND_Stream_Stop((FSOUND_STREAM*)(sound->handle));
		if(! res ){
			FSOUND_GetError();
		}
		break;
	default:
		// @@@todo@@@: log this
		return;
	}
}

void sound_play(SSound* sound, float pan, float theVolume, char loop){
	int volume = (int)(256*theVolume);
	if(! sound->used ) return;
	switch(sound->type){
	case TYPE_PCM:
		sound_stop(sound);
		sound->channel = FSOUND_PlaySound(FSOUND_FREE, (FSOUND_SAMPLE*) sound->handle);
		sound_setPanVol(sound, pan, theVolume);
		sound_setPaused(sound, 1); // 4 compatibility reasons - see fmod doc on FSOUND_SetLoopMode...
		FSOUND_SetLoopMode(sound->channel, ((loop)?FSOUND_LOOP_NORMAL:FSOUND_LOOP_OFF) );
		sound_setPaused(sound, 0);
		sound->position = 0;
		sound->min = 0.0f;
		sound->ignore = GE_FALSE;
		if( !sound->useChannel ){
			soundSystem_change[sound_nextIndex].used = 1;
			soundSystem_change[sound_nextIndex].channel = sound->channel;
			soundSystem_change[sound_nextIndex].min = 0.0f;
			soundSystem_change[sound_nextIndex].ignore = GE_FALSE;
			soundSystem_change[sound_nextIndex].use3d = GE_FALSE;
			//soundSystem_change[sound_nextIndex]
			sound_nextIndex++;
			if( sound_nextIndex >= SOUNDSYS_NUMBEROFSOUNDS ) sound_nextIndex = 0;
		}
		break;
	case TYPE_SEQ:
		sound->channel = -1;
		sound->position = 0;
		sound->min = 0.0f;
		sound->ignore = GE_FALSE;
		if( !loop ){
			FMUSIC_SetLooping((FMUSIC_MODULE*)(sound->handle), FALSE);
		}
		FMUSIC_PlaySong( (FMUSIC_MODULE*)(sound->handle) );
		FMUSIC_SetMasterVolume((FMUSIC_MODULE*)sound->handle, volume);
		break;
	case TYPE_STREAM:
		sound_stop(sound);

		if( loop ){
			//FSOUND_Stream_SetLoopCount((FSOUND_STREAM*) sound->handle,40 )
			if( FALSE == FSOUND_Stream_SetMode((FSOUND_STREAM*) sound->handle, FSOUND_LOOP_NORMAL ) ) {
				soundsys_logError("failed to set loop mode");
				return;
			}
		}

		sound->channel = FSOUND_Stream_Play(FSOUND_FREE, (FSOUND_STREAM*) sound->handle);
		sound_setPanVol(sound, pan, theVolume);
		
		sound->position = 0;
		sound->min = 0.0f;
		sound->ignore = GE_FALSE;
		if( !sound->useChannel ){
			soundSystem_change[sound_nextIndex].used = 1;
			soundSystem_change[sound_nextIndex].channel = sound->channel;
			soundSystem_change[sound_nextIndex].min = 0.0f;
			soundSystem_change[sound_nextIndex].ignore = GE_FALSE;
			soundSystem_change[sound_nextIndex].position;
			soundSystem_change[sound_nextIndex].use3d = GE_FALSE;
			//soundSystem_change[sound_nextIndex]
			sound_nextIndex++;
			if( sound_nextIndex >= SOUNDSYS_NUMBEROFSOUNDS ) sound_nextIndex = 0;
		}
		break;
	default:
		// @@@todo@@@: log this
		return;
	}
	sound->min = 0.0f;
	sound->ignore = GE_FALSE;
	sound->position = 0;
}
void sound_play3d(SSound* sound, float min, geBoolean ignore, geVec3d* position, char loop){
	int volume = 255;
	if(! sound->used ) return;
	switch(sound->type){
	case TYPE_PCM:
		sound_stop(sound);
		sound->channel = FSOUND_PlaySound(FSOUND_FREE, (FSOUND_SAMPLE*) sound->handle);
		sound_setPaused(sound, 1); // 4 compatibility reasons - see fmod doc on FSOUND_SetLoopMode...
		FSOUND_SetLoopMode(sound->channel, ((loop)?FSOUND_LOOP_NORMAL:FSOUND_LOOP_OFF) );
		sound_setPaused(sound, 0);
		sound->min = min;
		sound->ignore = ignore;
		sound->position = position;
		if( !sound->useChannel ){
			soundSystem_change[sound_nextIndex].used = 1;
			soundSystem_change[sound_nextIndex].channel = sound->channel;
			soundSystem_change[sound_nextIndex].min = min;
			soundSystem_change[sound_nextIndex].ignore = ignore;
			soundSystem_change[sound_nextIndex].position = *position;
			soundSystem_change[sound_nextIndex].use3d = GE_TRUE;
			//soundSystem_change[sound_nextIndex]
			sound_nextIndex++;
			if( sound_nextIndex >= SOUNDSYS_NUMBEROFSOUNDS ) sound_nextIndex = 0;
		}
		break;
	case TYPE_SEQ:
		if( !loop ){
			FMUSIC_SetLooping((FMUSIC_MODULE*)(sound->handle), FALSE);
		}
		FMUSIC_PlaySong( (FMUSIC_MODULE*)(sound->handle) );
		FMUSIC_SetMasterVolume((FMUSIC_MODULE*)sound->handle, volume);
		break;
	case TYPE_STREAM:
		sound_stop(sound);

		if( loop ){
			//FSOUND_Stream_SetLoopCount((FSOUND_STREAM*) sound->handle,40 )
			if( FALSE == FSOUND_Stream_SetMode((FSOUND_STREAM*) sound->handle, FSOUND_LOOP_NORMAL ) ) {
				soundsys_logError("failed to set loop mode");
				return;
			}
		}

		sound->channel = FSOUND_Stream_Play(FSOUND_FREE, (FSOUND_STREAM*) sound->handle);
		
		sound->min = min;
		sound->ignore = ignore;
		sound->position = position;

		if( !sound->useChannel ){
			soundSystem_change[sound_nextIndex].used = 1;
			soundSystem_change[sound_nextIndex].channel = sound->channel;
			soundSystem_change[sound_nextIndex].min = min;
			soundSystem_change[sound_nextIndex].ignore = ignore;
			soundSystem_change[sound_nextIndex].position = *position;
			soundSystem_change[sound_nextIndex].use3d = GE_TRUE;
			//soundSystem_change[sound_nextIndex]
			sound_nextIndex++;
			if( sound_nextIndex >= SOUNDSYS_NUMBEROFSOUNDS ) sound_nextIndex = 0;
		}
		break;
	default:
		// @@@todo@@@: log this
		return;
	}
	sound->modType = TYPE_3D;
}

char fexist(char* file){
	FILE *f = fopen(file, "rb");
	if( f ) {
		fclose(f);
		return 1;
	}
	return 0;
}

char sound_loadPcm(SSound* sound, char* file, char useChannel){
	if( !fexist(file) ) return 0;
	sound->handle = FSOUND_Sample_Load(FSOUND_FREE, file, FSOUND_NORMAL, 0, 0);
	sound->type =TYPE_PCM;
	sound->used = 1;
	sound->useChannel = useChannel;
	if( sound->handle ) return 1;
	else return 1;
}
char sound_loadSequenced(SSound* sound, char* file){
	if( !fexist(file) ) return 0;
	sound->handle = FMUSIC_LoadSong(file);
	sound->type =TYPE_SEQ;
	sound->used = 1;
	sound->useChannel = 0;
	if( sound->handle ) return 1;
	else return 0;
	return 1;
}
char sound_loadStream(SSound* sound, char* file, char useChannel){
	if( !fexist(file) ) return 0;
	sound->handle = FSOUND_Stream_Open(file, 0, 0, 0);
	sound->type =TYPE_STREAM;
	sound->used = 1;
	sound->useChannel = useChannel;
	if( sound->handle ) return 1;
	else return 0;
	return 1;
}
void sound_delete(SSound* sound){
	if(! sound->used ) return;
	switch(sound->type){
	case TYPE_PCM:
		FSOUND_Sample_Free( (FSOUND_SAMPLE*)(sound->handle) );
		sound->used = 0;
		break;
	case TYPE_SEQ:
		FMUSIC_FreeSong( (FMUSIC_MODULE*)(sound->handle) );
		break;
	case TYPE_STREAM:
		FSOUND_Stream_Close( (FSOUND_STREAM*) (sound->handle) );
		break;
	default:
		// @@@todo@@@: log this
		return;
	}
	ssound_clear(sound); // remove all data from it, so we won't accedentently delete it again
}

/*
#define TYPE_LEFT	0
#define TYPE_RIGHT	1
#define TYPE_BOTH	2
#define SOUNDSYS_DEFAULT_VOLUME					1.0f
#define SOUNDSYS_DEFAULT_PAN					0.0f
#define SOUNDSYS_DEFAULT_FREQUENCY				1.0f
#define SOUNDSYS_NUMBEROFSOUNDS					200
#define SOUNDSYS_NUMBEROFTEMPORARYS				3
*/

void soundsys_clearTemporaries(){
	int i = 0;
	for( i=0; i< SOUNDSYS_NUMBEROFTEMPORARYS; i++){
		ssound_clear(&(soundSystem_temp[i]));
	}
}

void soundsys_internalInit(){
	int i = 0;
	soundSys_nextIndex = 0;
	sound_nextIndex = 0;
	for( i=0; i< SOUNDSYS_NUMBEROFSOUNDDEFS; i++){
		ssound_clear(&(soundSystem_sounds[i]));
	}
	for( i=0; i< SOUNDSYS_NUMBEROFSOUNDS; i++){
		schangeable_clear(&(soundSystem_change[i]));
	}
	ssound_clear(&playerSoundDef);
	ssound_clear(&songDef);
	soundsys_clearTemporaries();
}

void soundsys_freeTemporaries(){
	int i = 0;
	for( i=0; i< SOUNDSYS_NUMBEROFTEMPORARYS; i++){
		sound_delete(&(soundSystem_temp[i]));
	}
	sound_delete(&playerSoundDef);
	sound_delete(&songDef);
}

geBoolean soundsys_load( HWND hWnd){
	soundsys_internalInit();

	if (FSOUND_GetVersion() < FMOD_VERSION) {
        //printf("Error : You are using the wrong DLL version!  You should be using FMOD %.02f\n", FMOD_VERSION);
		soundsys_logError("You are using the wrong DLL version of FMod!");
        return GE_FALSE;
    }

    if (!FSOUND_Init(32000, 32, 0)) {
        //printf("%s\n", FMOD_ErrorString(FSOUND_GetError()));
		soundsys_logError("Failed to init fmod");
        return GE_FALSE;
    }

	// FSOUND_SetHWND(hWnd);

	return GE_TRUE;
}

void soundsys_destroy(){
	int i = 0;
	for( i=0; i< SOUNDSYS_NUMBEROFSOUNDS; i++){
		sound_delete( &(soundSystem_sounds[i]) );
	}
	FSOUND_Close();
}

geBoolean soundsys_play3dsound(soundsys_sound *id, geVec3d *position, float min, geBoolean ignore, char loop){
	SSound* sound = &(soundSystem_sounds[id->id]);
	if(! sound->used ){
		return GE_FALSE;
	}
	sound_play3d(sound, min, ignore, position,loop);
	return GE_TRUE;
}

geBoolean soundsys_play_sound(soundsys_sound *id, geBoolean loop, char typeHandle, float pan, float volume){
	if( soundSystem_sounds[id->id].useChannel ){
		if( sound_isPlaying( &(soundSystem_sounds[id->id]) ) ){
			switch( typeHandle ){
			case TYPE_PLAY:
				break;
			case TYPE_STOP:
				sound_stop( &(soundSystem_sounds[id->id]) );
				break;
			case TYPE_FAIL:
			default:
				return GE_FALSE;
			}
		}
	}
	sound_play(&(soundSystem_sounds[id->id]),
		SETUP_PAN(pan),
		volume,
		(loop==GE_TRUE)?1:0);
	return GE_TRUE;
}

geBoolean soundsys_play_sound_ex(soundsys_sound *id, geBoolean loop, char type){
	float pan = 0.0f;
	switch( type ){
	case TYPE_LEFT:
		pan = -1.0f;
		break;
	case TYPE_RIGHT:
		pan = 1.0f;
		break;
	case TYPE_BOTH:
	default:
		pan = 0.0f;
		break;
	}
	soundsys_play_sound(id, loop, TYPE_STOP, pan, 1.0f); // the pan is converted in play_sound function, do not do this here.
	return GE_TRUE;
}

geBoolean soundsys_loadSound(char* file, char sample, soundsys_sound *sound, char useChannel, char typePlay){
	char res = 0;

	if( soundSys_nextIndex >= SOUNDSYS_NUMBEROFSOUNDDEFS ){
		printLog("***************************************** Failed to load sound file, increase NUNMBEROFSOUNDDEFS macro\n");
		sound->id = 0;
		return GE_FALSE;
	}


	if( isSequencedBased(file) ){
		if( sample ){
			res = sound_loadSequenced(&(soundSystem_sounds[soundSys_nextIndex]), file);
			if ( res ){
				sound->id = soundSys_nextIndex;
			}
			else {
				sound->id = 0;
				return failedToLoadSound(file);
			}
		}
		else {
			return failedToLoadSound(file);
		}
	}
	else {
		if( sample ){
			res = sound_loadPcm(&(soundSystem_sounds[soundSys_nextIndex]), file, useChannel);
			if ( res ){
				sound->id = soundSys_nextIndex;
			}
			else {
				sound->id = 0;
				return failedToLoadSound(file);
			}
		}
		else {
			res = sound_loadStream(&(soundSystem_sounds[soundSys_nextIndex]), file, useChannel);
			if ( res ){
				sound->id = soundSys_nextIndex;
			}
			else {
				sound->id = 0;
				return failedToLoadSound(file);
			}
		}
	}

	soundSystem_sounds[soundSys_nextIndex].modType = ((typePlay==TYPE_2D)?(TYPE_2D):(TYPE_UNAFFECTED)); // make sure it's withing valid values - no 3d here
	soundSystem_sounds[soundSys_nextIndex].useChannel = !useChannel;
	soundSystem_sounds[soundSys_nextIndex].file = file;
	soundSystem_sounds[soundSys_nextIndex].position = 0;
	soundSystem_sounds[soundSys_nextIndex].min = 0.0f;
	soundSystem_sounds[soundSys_nextIndex].ignore = GE_FALSE;

	soundSys_nextIndex++;
	return GE_TRUE;
}

geBoolean soundsys_play_once_sound(char* psound){
	SSound* sound;
	if( sound_tempIndex >= SOUNDSYS_NUMBEROFTEMPORARYS) sound_tempIndex=0;
	sound = &(soundSystem_temp[sound_tempIndex]);

	if( sound->used ){
		if( sound_isPlaying(sound) ) return GE_FALSE;
		sound_delete(sound);
	}
	if(! sound_loadStream(sound, psound, 0) ) return failedToLoadSound(psound);
	sound_play(sound, 0.0f, 1.0f, 0);

	sound_tempIndex++;
	return GE_TRUE;
}
geBoolean soundsys_play_once_3dsound(char* psound, geVec3d *position, float min, geBoolean ignore){
	SSound* sound;
	if( sound_tempIndex >= SOUNDSYS_NUMBEROFTEMPORARYS) sound_tempIndex=0;
	sound = &(soundSystem_temp[sound_tempIndex]);

	if( sound->used ){
		if( sound_isPlaying(sound) ) return GE_FALSE;
		sound_delete(sound);
	}
	if(! sound_loadStream(sound, psound, 0) ) return failedToLoadSound(psound);
	sound_play3d(sound, min, ignore, position, 0);

	sound_tempIndex++;
	return GE_TRUE;
}
geBoolean soundsys_conversation_isTalking(){
	if( !playerSoundDef.used ) return GE_FALSE;
	return sound_isPlaying(&playerSoundDef);
}
void soundsys_conversation_stopTalkiing(){
	if( !playerSoundDef.used ) return;
	sound_stop( &playerSoundDef );
}
geBoolean soundsys_conversation_heroSay(char* soundFile){
	if( playerSoundDef.used ) {
		sound_delete(&playerSoundDef);
	}
	if(! sound_loadStream(&playerSoundDef, soundFile, 1) ) return failedToLoadSound(soundFile);
	sound_play(&playerSoundDef, -10.0f, 1.0f, 0);
	return GE_TRUE;
}
geBoolean soundsys_conversation_personSay(char* sound, geVec3d *position, float min, geBoolean ignore){
	if( playerSoundDef.used ) {
		sound_delete(&playerSoundDef);
	}
	if(! sound_loadStream(&playerSoundDef, sound, 1) ) return failedToLoadSound(sound);
	if( position ){
		sound_play3d(&playerSoundDef, min, ignore, position, 0);
	}
	else {
		sound_play(&playerSoundDef, 0.0, 1.0f, 0);
	}
	return GE_TRUE;
}
void soundsys_conversation_updatePersonSound(const geVec3d* position, float min, geBoolean ignore){
}
geBoolean soundsys_master(geFloat master){
	int volume;
	if( master > 1.0f ) return GE_FALSE;
	if( master < 0.0f ) return GE_FALSE;

	volume = (int)(master * 255.0f);

	FSOUND_SetSFXMasterVolume(volume);

	return GE_TRUE;
}

void soundsys_stop(soundsys_sound *sound){
	sound_stop( &(soundSystem_sounds[sound->id]) );
}
void soundsys_updateSound(float time){
	setFreqAll(time);
}
void soundsys_setGameVolume(float vol){
	if( vol < 0.0f ) return;
	if( vol > 10.0f ) return;
	volume = vol;
	setVolumeAll();
}

geBoolean soundsys_playMusic(char* file){
	if( songDef.used ) soundsys_stopMusic();
	if(! sound_loadStream(&songDef, file, 1) )
		return failedToLoadSound(file);
	sound_play(&songDef, 0.0f, musicVolume, 1);
	return GE_TRUE;
}
void soundsys_stopMusic(){
	sound_stop(&songDef);
	sound_delete(&songDef);
	songDef.used = 0; // to make sure since other functions depend on this var!
}

geBoolean soundsys_changeMusic(char* file){
	SSound temp;
	int ms;
	int maxMs;
	if(! songDef.used ) return soundsys_playMusic(file);
	
	if(! sound_loadStream(&temp, file, 1) )
		return failedToLoadSound(file);

	ms = FSOUND_Stream_GetTime((FSOUND_STREAM*) songDef.handle );
	maxMs = FSOUND_Stream_GetLengthMs((FSOUND_STREAM*) temp.handle );
	if( ms > maxMs ){
		console_message("Failed to change music since the position is greater than the new length.");
		soundsys_logError("Soundsys: Failed to change music since the position is greater than the new length.");
		return GE_FALSE;
	}

	soundsys_stopMusic();
	songDef = temp;

	sound_play(&songDef, 0.0f, musicVolume, 1);
	// set file positition
	if( FSOUND_Stream_SetTime((FSOUND_STREAM*) songDef.handle, ms) == FALSE ){
		console_message("Failed to set the position in the new music");
		soundsys_logError("Soundsys: Failed to set the position in the new music");
		return GE_FALSE;
	}

	return GE_TRUE;
}