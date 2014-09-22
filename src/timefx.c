#include "timefx.h"
#include "sound_system.h"

void timefx_slowmotion( geFloat fx )
{
	if( fx < 1.0f ) return;
	enemyTime =		1/fx;
	heroTime =		1/fx;
	gravityTime =	1/fx;
	soundsys_updateSound(heroTime, enemyTime);
}

void timefx_drug( geFloat fx )
{
	if( fx < 1.0f ) return;
	enemyTime =		1/fx;
	heroTime =		1.0f;
	gravityTime =	1/fx;
	soundsys_updateSound(fx, enemyTime);
}

void timefx_bite( geFloat fx )
{
	if( fx < 1.0f ) return;
	enemyTime =		1.0f;
	heroTime =		1/fx;
	gravityTime =	1/fx;
	soundsys_updateSound(heroTime, fx);
}

void timefx_normal()
{
	enemyTime =		1.0f;
	heroTime =		1.0f;
	gravityTime =	1.0f;
	soundsys_updateSound(heroTime, enemyTime);
}