#include "jacktalks.h"
#include "sound_system.h"
#include "playerSounds.h"

void mugshot()
{
	soundsys_stop( &mugshots[ 0 ] );
	soundsys_stop( &mugshots[ 1 ] );
	soundsys_stop( &mugshots[ 2 ] );
	soundsys_stop( &mugshots[ 3 ] );
	soundsys_play_sound( &mugshots[ rand()%4 ], GE_FALSE, TYPE_STOP, SOUND_SURROUND, 1.0f );
}

float talkPower = 0.0f;
void talkPower_Change(float extraTP)
{
	talkPower += extraTP;

	if( talkPower < 0.0f ) talkPower = 0.0f;

	if( talkPower > 10.0f )
	{
		talkPower = 0.0f;
		mugshot();
	}
}