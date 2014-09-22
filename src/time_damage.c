#include "time_damage.h" // we will define theese functions declared in this file

#include "memory.h"
#include "genesis.h"
#include "player.h" // for the moved variable
#include "console.h" // debug and error printing
#include <stdlib.h>
#include "timefx.h"
#include "globalGenesis.h"
#include "damage.h"
#include "log.h"
/*
#define TIMEDAMAGE_ZOMBIEBITE 0
#define TIMEDAMAGE_LAVADAMAGE 1
#define TIMEDAMAGE_INFECTEDBITE 2
#define TIMEDAMAGE_FIRE 3
#define TIMEDAMAGE_DRUG 4
#define TIMEDAMAGE_SIZE 5
*/

char td_predone=0;

typedef struct timeDamage_tag
{
	float timeLeft; // time left for the next damage
	float maxTime; // total time betwen the damages
	unsigned int damageLeft; // when zero this td has no effect
	unsigned char damage; // the damage
	char ignoreArmor; // ignore armor when dammage?
	char move; // does moves affect this damage, ie will running slow things down?
	float moveTime;
	float maxMove;
	float fov; // the foc that the td changes to

	// timefx
	float timefx_value; // the value
	char timefx_type; // the type
} TimeDamage;

TimeDamage td[TIMEDAMAGE_SIZE];

int handle_timeDamage(SaveFile* file){
	int i=0;

	for( i=0; i< TIMEDAMAGE_SIZE; i++){
		FLOAT( td[i].timeLeft, "Failed to handle timedamage timeLeft" );
		INT( td[i].damageLeft , "Failed to handle damage left");
	}

	return 1;
}

void timedamage_init()
{
	TimeDamage* c=0;
	memset(&td, 0, sizeof(TimeDamage) );


	c = &( td[TIMEDAMAGE_ZOMBIEBITE] );
	c->maxTime = 3.0f;
	c->damage = 10;
	c->ignoreArmor = GE_TRUE;
	c->fov = 6.0f;
	c->timefx_type = 1;
	c->timefx_value = 2;

	c = &( td[TIMEDAMAGE_LAVADAMAGE] );
	c->maxTime = 1.0f;
	c->damage = 25;
	c->ignoreArmor = GE_FALSE;
	c->fov = 2.0f;
	c->timefx_type = 0;
	c->timefx_value = 1;

	c = &( td[TIMEDAMAGE_INFECTEDBITE] );
	c->maxTime = 5.0f;
	c->damage = 5;
	c->ignoreArmor = GE_TRUE;
	c->fov = 4.0f;
	c->timefx_type = 1;
	c->timefx_value = 1.5f;

	c = &( td[TIMEDAMAGE_FIRE] );
	c->maxTime = 0.8f;
	c->damage = 10;
	c->ignoreArmor = GE_FALSE;
	c->fov = 2.0f;
	c->timefx_type = 0;
	c->timefx_value = 1;

	c = &( td[TIMEDAMAGE_DRUG] );
	c->maxTime = 10.0f;
	c->damage = 3;
	c->ignoreArmor = GE_TRUE;
	c->fov = 3.0f;
	c->timefx_type = 2;
	c->timefx_value = 3;
}

void timedamage_clear(){
	int i=0;
	for(i=0; i< TIMEDAMAGE_SIZE; i++){
		td[i].timeLeft = 0.0f;
		td[i].damageLeft = 0;
	}
}

void timedamage_add( unsigned char type )
{
	if( type >= TIMEDAMAGE_SIZE ) return;
	td[type].damageLeft += 5;
}

char timedamage_has( unsigned char type )
{
	if( type >= TIMEDAMAGE_SIZE ) return 0;
	return ( td[type].damageLeft > 0 );
}

void timedamage_iterate()
{
	unsigned char i;

	// standard values, no other values are allowed below theese
	char timefx_type=0;
	float timefx_value=1.0f;
	float fov=2.0f;
	char done=0;

	for(i=0; i<TIMEDAMAGE_SIZE; i++ )
	{
		if( td[i].damageLeft )
		{
			done = 1;
			// check damage
			if( td[i].timeLeft <= 0.0f )
			{
				unsigned char theDamage = rand()%td[i].damage +1;
				td[i].timeLeft = td[i].maxTime;
				if( td[i].ignoreArmor )
					armor_piercing_damage( theDamage );
				else
					damage( theDamage );
				td[i].damageLeft--;
			}

			if( td[i].fov > fov ) fov = td[i].fov;
			if( td[i].timefx_type > timefx_type ) timefx_type = td[i].timefx_type;
			if( td[i].timefx_value > timefx_value ) timefx_value = td[i].timefx_value;

			//can we move away from damage?
			if( td[i].move)
			{
				if( moved )
					td[i].moveTime -= TIME * heroTime; // if we have moved the remember it
				else if( td[i].moveTime < td[i].maxMove )
					td[i].moveTime += TIME* heroTime; // if we don't move then we'll continue and try to disort that

				// have we moved enough ?
				if( td[i].moveTime < 0.0f )
				{
					// then make use of it
					td[i].moveTime += td[i].maxMove;
					td[i].damageLeft--;
				}
			}

			td[i].timeLeft -= TIME ;//* heroTime;
		}
	}

	if( done )
	{
		switch( timefx_type )
		{
		case 0:
			// nothing
			break;
		case 1:
			timefx_bite(timefx_value);
			break;
		case 2:
			timefx_drug(timefx_value);
			break;
		}
		geCamera_SetAttributes(Camera, fov, &Rect);

		td_predone = done;
	}
	else
	{
		if( done!= td_predone )
		{
			geCamera_SetAttributes(Camera, 2.0f, &Rect); // restore the fov
			timefx_normal(); // restore the timefx
			td_predone = done;
		}
	}
}