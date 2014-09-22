#include "timefx.h"
#include "sound_system.h"

void timefx_slowmotion( geFloat fx )
{
	if( fx < 1.0f ) return;
	enemyTime =		1/fx;
	heroTime =		1/fx;
	gravityTime =	1/fx;
}

void timefx_drug( geFloat fx )
{
	if( fx < 1.0f ) return;
	enemyTime =		1/fx;
	heroTime =		0.9f;
	gravityTime =	1/fx;
}

void timefx_bite( geFloat fx )
{
	if( fx < 1.0f ) return;
	enemyTime =		fx;
	heroTime =		fx/2.0f;
	gravityTime =	fx/2.0f;
}

void timefx_normal()
{
	enemyTime =		1.0f;
	heroTime =		1.0f;
	gravityTime =	1.0f;
}