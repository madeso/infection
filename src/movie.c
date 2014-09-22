#include "genesis.h"
#include "extra_genesis.h"
#include "init.h"
#include "globalGenesis.h"
#include "log.h"
#include "sound_system.h"

geBoolean playMovieSound(const char* snd){
	char str[300];
	sprintf(str, ".\\levels\\%s\\Movies\\sfx\\%s.wav", getLevelFileName(), snd);
	soundsys_conversation_stopTalkiing();
	soundsys_conversation_heroSay(str);
	return GE_TRUE;
}

// shows the initializing infection image
geBoolean displayMovieImage(const char* img)
{
	geBitmap *image=0;
	char str[300];
	sprintf(str, ".\\levels\\%s\\Movies\\gfx\\%s.bmp", getLevelFileName(), img);

	image = loadBitmapEx(str, Width, Height, Engine);

	if( !image )
	{
		printLog("Failed to load the image: ");
		printLog(str);
		printLog("!!!!!\n");
	}

	if( geEngine_BeginFrame(Engine, Camera, GE_FALSE) == GE_FALSE )
	{
		printLog("***geEngine_BeginFrame() failed.\n\n");
		return GE_FALSE;
	}
	
	if( !geEngine_DrawBitmap(Engine, image, NULL, 0, 0) )
	{
		printLog("Failed to draw image.\n");
		return GE_FALSE;
	}

	//End frame
	if( geEngine_EndFrame(Engine) == GE_FALSE )
	{
		printLog("***geEngine_EndFrame() failed.\n\n");
		return GE_FALSE;
	}

	if( image )
	{
		printLog("Removing image.\n");
		geEngine_RemoveBitmap(Engine, image);
		geBitmap_Destroy(&image);
	}

	return GE_TRUE;
}


geBoolean movie_execute(const char* movieImage, const char* movieSound, float movieTime) {
	float now = 0.0f;
	float end = 0.0f;
	now = (float)timeGetTime()/1000.0f;
	end = now + movieTime;
	if(! displayMovieImage(movieImage) ) return GE_FALSE;
	if(! playMovieSound(movieSound) ) return GE_FALSE;

	while( end > now ) {
		now = (float)timeGetTime()/1000.0f;
	}

	// fix things to gameplay
	reset_cursor(); // so no sudden view change is made
	timing_reset(); // timing is everything

	return GE_TRUE;
}