#include <math.h>

#include "genesis.h"
#include "player.h"
#include "inf_message_system.h"
#include "useful_macros.h"
#include "keys.h"
#include "time_damage.h"
#include "sound_system.h"

#include "globalGenesis.h"
#include "playerSounds.h"
#include "damage.h"
#include "log.h"
#include "weapons.h"
#include "globals.h"
#include "timefx.h"

#include "jacktalks.h"
#include "level.h"

#include "entity_iterate.h"
#include "infection.h"

#define STEP_SIZE 45

//ladder
geVec3d inLadder;

//#define MULTIPLE_JUMPS 3

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// flashlight local data
geLight* flashlightLight=0;
GE_RGBA flashlightRgb;
geFloat radius = 250.0f;
geBoolean flashLightOn = GE_FALSE;
geVec3d flashPos;
#define FLASHLIGHT_ON()			if(!flashlightLight)	flashlightLight = geWorld_AddLight(World)
#define FLASHLIGHT_OFF()		if( flashlightLight)	{ geWorld_RemoveLight(World, flashlightLight); flashlightLight=0;}
#define UPDATE_FLASHLIGHT()		if( flashlightLight)	geWorld_SetLightAttributes( World, flashlightLight, &flashPos, &flashlightRgb, radius, GE_TRUE)
///////////////////////////////////////////////////////////////////////////////////////////////////////////

geBoolean flashkeyDown = GE_FALSE;
geBoolean usekeyDown = GE_FALSE;
geBoolean qsaveDown = GE_FALSE;
geBoolean qloadDown = GE_FALSE;

#ifdef MULTIPLE_JUMPS
char player_jumps = 0;
#endif

#define TRACE_DISTANCE 100.0f
void player_use(){
	GE_Collision Col;
	geVec3d In;
	geVec3d farPos;

	geXForm3d_GetIn(&XForm,  &In);
	//geCamera_ScreenPointToWorld(Camera, Width/2, Height/2, &In);
	geVec3d_AddScaled(&(XForm.Translation), &In, TRACE_DISTANCE, &farPos); 

	if( geWorld_Collision(
		World,
		NULL,
		NULL,
		&( XForm.Translation ),
		&farPos,
		GE_CONTENTS_SOLID_CLIP,
		GE_COLLIDE_ALL,
		0xffffffff,
		NULL,
		NULL,
		&Col
		)//end of function call
		)//end of if
	{
		int result = 0;

		if(! Col.Model ){
			return;
		}

		result = use_entity(Col.Model);

		switch( result ){
		case ENTITY_NO_USABLE_ENTETIES:
			//game_message("You don't know how to operate on that entity");
			break;
		case ENTITY_ERROR:
			system_message("Error from use entity(Col.Model)");
			break;
		case ENTITY_DISABLED:
			game_message("That entity is disabled");
			break;
		case ENTITY_RUN:
			//game_message("You used entity");
			break;
		}
	}
}
#undef TRACE_DISTANCE

#define TRACE_DISTANCE 4500.0f
void trace_flashlight()
{
	GE_Collision Col;
	geVec3d In;

	geXForm3d_GetIn(&XForm,  &In);
	//geCamera_ScreenPointToWorld(Camera, Width/2, Height/2, &In);
	geVec3d_AddScaled(&(XForm.Translation), &In, TRACE_DISTANCE, &flashPos); 

	if( geWorld_Collision(
		World,
		NULL,
		NULL,
		&( XForm.Translation ),
		&flashPos,
		GE_CONTENTS_SOLID_CLIP,
		GE_COLLIDE_ALL,
		0xffffffff,
		NULL,
		NULL,
		&Col
		)//end of function call
		)//end of if
	{
		// set to a fix distance away from the wall, so we are sure the light isn't in the wall
		// flashPos = Col.Impact + Col.Plane.Normal * 5.0f;
		geVec3d_AddScaled( & (Col.Impact), &( Col.Plane.Normal ), 5.0f, &flashPos );
	}
}
#undef TRACE_DISTANCE

void player_end()
{
	FLASHLIGHT_OFF();
}

void player_start()
{
	trace_flashlight();
	flashlightRgb.r = 200.0f;
	flashlightRgb.g = 200.0f;
	flashlightRgb.b = 200.0f;
	flashlightRgb.a = 200.0f;


	if( flashLightOn )
	{
		FLASHLIGHT_ON();
		UPDATE_FLASHLIGHT();
	}
}

// this function is currently only called if the consoel isn't activated
void player_frame()
{
	if( flashLightOn )
	{
		trace_flashlight();
		UPDATE_FLASHLIGHT();
	}
}

void flip_flashlight()
{
	//system_message("flip_flashlight()");
	if( flashLightOn ) // if the flashlight is on..
	{
		// deactivate the flashlight
		FLASHLIGHT_OFF();
		flashLightOn = GE_FALSE;
		//game_message("Deactivating flashlight");
	}
	else
	{
		// activate flashlight
		FLASHLIGHT_ON();
		UPDATE_FLASHLIGHT();
		flashLightOn = GE_TRUE;
		//game_message("Activating flashlight");
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//		player_is_on_ground
//			returns true if the hero is on the the ground, false if not
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

geBoolean player_is_on_ground()
{
	//check for collision from current position XForm.translation and 1 unit down
	geVec3d Pos;
	geVec3d_Set(&Pos,
		XForm.Translation.X,
		XForm.Translation.Y-0.5f,
		XForm.Translation.Z
		);
	
	if	(geWorld_Collision(
		World,					//The world
		&ExtBox.Min,			//Boundingbox min
		&ExtBox.Max,			//Boundingbox max
		&XForm.Translation,		//Old pos
		&Pos,					//Future pos
		GE_CONTENTS_SOLID_CLIP,	//Collission type
		GE_COLLIDE_ALL,			//Collission type
		0xffffffff,				//?
		cb_move,				//?
		NULL,					//?
		&Col					//We need to save the collission info data
		)//end of function call
		)//end of if
	{
		return GE_TRUE;
	}
	
	return GE_FALSE;
}

geBoolean player_canStand(geFloat change)
{
	//check for collision from current position XForm.translation and 1 unit down
	geVec3d nPos;
	nPos = Pos;
	nPos.Y = Pos.Y + change;
	
	if	(geWorld_Collision(
		World,					//The world
		&ExtBox.Min,			//Boundingbox min
		&ExtBox.Max,			//Boundingbox max
		&Pos,					//Old pos
		&nPos,					//Future pos
		GE_CONTENTS_SOLID_CLIP,	//Collission type
		GE_COLLIDE_ALL,			//Collission type
		0xffffffff,				//?
		cb_move,				//?
		NULL,					//?
		&Col					//We need to save the collission info data
		)//end of function call
		)//end of if
	{
		return GE_FALSE;
	}
	
	return GE_TRUE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void update_player_state()
{
	if (geWorld_GetContents(
		World,
		&XForm.Translation,
		&stateBox.Min,
		&stateBox.Max,
		GE_COLLIDE_MODELS,
		0,
		NULL,
		NULL,
		&contents
		) // func call
		) //if
    { 
        // Check to see if player is in lava... 
        if (contents.Contents & CONTENTS_WATER) 
        { 
			player_state = STATE_WATER;
        } 
        else if (contents.Contents & CONTENTS_LADDER) 
        { 
			player_state = STATE_LADDER;
        } 
        else
        { 
			player_state = STATE_NORMAL;
        }
    }
	else
	{
		player_state = STATE_NORMAL;
	}
}

unsigned char player_getState()
{
	//update_player_state();
	return player_state;
}

void player_getPos(geVec3d *pos)
{
	float diff = 0.0f;

	diff = -1.0f*( 135.0f - FLOAT_ABS(ExtBox.Min.Y) );
	*pos = Pos;
	pos->Y -= diff;
}

float player_getPlayerFloor()
{
	geVec3d floor;
	player_getPos(&floor);
	return floor.Y;
}

void cleanUpStateBox(){
	if( ExtBox.Min.Y < -135.0f ){
		ExtBox.Min.Y = -135.0f;
	}

	if( ExtBox.Min.Y > -80.0f ){
		ExtBox.Min.Y = -80.0f;
	}
}

void player_crouchOrStand()
{
#define		COLLISION		geWorld_Collision(World,&ExtBox.Min,&ExtBox.Max,&fromPos,&Pos,GE_CONTENTS_SOLID_CLIP,GE_COLLIDE_ALL,0xffffffff,cb_move,NULL,&lCol)
	geVec3d fromPos;
	geFloat change = 450.0f * TIME * heroTime;
	GE_Collision lCol;
	
	fromPos = Pos;

	//if( !player_is_on_ground() ) return;
	
	if( player_getState() == STATE_WATER )
	{
		// swim down ?
		// climp dowm if on ladder ?
		return;
	}
	
	if( keys[ controls.crouch ] && player_is_on_ground() )
	{
		if( ExtBox.Min.Y < -80.0f )
		{
			//ExtBox.Min.Y += change;
			
			Pos.Y -= change;
			EXTBOX_SET(ExtBox.Min.Y + change);

			cleanUpStateBox();
			
			if( COLLISION )
			{
				//system_message("stand/crouch col");
				Pos = lCol.Impact;
				return;
			}
			//change +=  15.0f * TIME * heroTime;
		}
	}
	else
	{
		// make the player stand up directly
		if( !player_is_on_ground() )
		{
			change = -1.0f* ( 135.0f - FLOAT_ABS(ExtBox.Min.Y) );
			if( player_canStand(change) )
			{
				//char str[200];
				//sprintf(str, "a: %f", change);
				//game_message(str);
				if( ExtBox.Min.Y > -135.0f )//&& ExtBox.Min.Y < -70.0f)
				{
					Pos.Y += change;
					
					//game_message("b");

					if( COLLISION )
					{
						//system_message("crouch/stand col");
						Pos = lCol.Impact;
						return;
					}
					
					EXTBOX_SET(ExtBox.Min.Y - change);
					cleanUpStateBox();
				}
			}

		}
		else
		{
			if( player_canStand(change) )
			{
				if( ExtBox.Min.Y > -135.0f )//&& ExtBox.Min.Y < -70.0f)
				{
					Pos.Y += change;
					
					if( COLLISION )
					{
						//system_message("crouch/stand col");
						Pos = lCol.Impact;
						return;
					}
					
					EXTBOX_SET(ExtBox.Min.Y - change);
					cleanUpStateBox();
				}
			}
		}
	}
#undef COLLISION
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//		StepUp
//			Handles walking in stairs
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

geVec3d front;
char StepUp(geVec3d *cPos)
{
#define		COLLISION		geWorld_Collision(World,&ExtBox.Min,&ExtBox.Max,&fromPos,&toPos,GE_CONTENTS_SOLID_CLIP,GE_COLLIDE_ALL,0xffffffff,cb_move,NULL,&lCol)
	geVec3d fromPos;
	geVec3d toPos;
	//geVec3d front;
	GE_Collision lCol;
	geFloat multi;

	fromPos.X = toPos.X = cPos->X;
	fromPos.Z = toPos.Z = cPos->Z;
	fromPos.Y = Pos.Y; toPos.Y = cPos->Y + STEP_SIZE;
	
	
	if( COLLISION )
	{
		toPos.Y = lCol.Impact.Y;
	}
	
	fromPos.Y = toPos.Y;
	
	//
	// Move forward
	//
	
	multi = 1.0f;

	fromPos = toPos;

	if( player_getState() == STATE_NORMAL )
		front.Y = 0.0f;
	geVec3d_AddScaled(&toPos, &front, multi, &toPos);
	
	
	if( COLLISION )
	{
		return 1;
		//toPos.X = lCol.Impact.X;
		//toPos.Z = lCol.Impact.Z;
	}
	
	fromPos.X = toPos.X;
	fromPos.Z = toPos.Z;
	fromPos.Y = toPos.Y;
	
	//
	// Move down
	//
	//*
	Pos.Y -= STEP_SIZE; // or STEP_SIZE
	
	
	if( COLLISION )
	{
		toPos = lCol.Impact;
	}
	//*/
	
	
	// stepping successfull, update the Pos;
	

	cPos->X = toPos.X;
	cPos->Y = toPos.Y;
	cPos->Z = toPos.Z;
	
	
	return 0;
#undef COLLISION
}


void reset_cursor()
{
	printExtended("Reseting the cursor.\n");	
	SetCursorPos( Width/2, Height/2); //Reset the cursor
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//		user_interaction
//			updates the player state based on the global interaction variables
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void user_interaction()
{
	//-------------
	//Mouse looking
	//-------------
	
	geFloat rightleft_speed;
	geFloat updown_speed;
	
	GetCursorPos(&MousePos);
	
	rightleft_speed = abs(MousePos.x - Width/2) * mouse_x_sensitivity * TIME;
	updown_speed = abs(MousePos.y - Height/2) * mouse_y_sensitivity * TIME;

	geVec3d_Clear(&CurrentSpeed );

	weapon_tick();

	if( player_is_alive)// hero_hit_points > 0 )
	{
		
		//update mouse cheking
		//--------------------
		{
			int temp;
			
			temp = GetAsyncKeyState(controls.primaryFire) & 0x8000;
			mbnew[0] = ( mouse[0] != temp && temp!= 0) ? GE_TRUE : GE_FALSE;
			mouse[0] = temp;
			
			temp = GetAsyncKeyState(VK_MBUTTON) & 0x8000;
			mbnew[1] = ( mouse[1] != temp && temp!= 0 ) ? GE_TRUE : GE_FALSE;
			mouse[1] = temp;
			
			temp = GetAsyncKeyState(controls.secondaryFire) & 0x8000;
			mbnew[2] = ( mouse[2] != temp && temp!= 0 ) ? GE_TRUE : GE_FALSE;
			mouse[2] = temp;
		}
		
		
		//--------------------------------------------------------------------------------------------------------------------
		
		if(MousePos.x < Width/2) //Turn Left
		{
			Angle.Y += rightleft_speed;
			if(Angle.Y >= 360.0f )
				Angle.Y -= 360.0f;
		}
		
		if(MousePos.x > Width/2) //Turn Right
		{
			Angle.Y -= rightleft_speed;
			if( Angle.Y < 0.0f)
				Angle.Y += 360.0f;
		}
		
		if(MousePos.y < Height/2) //Look Up
		{
			Angle.X += updown_speed;
			
			if( Angle.X > 80.0f)
			{
				Angle.X = 80.0f;
			}
			
			//if( Angle.X >= 360.0f )
			//	Angle.X -= 360.0f;
		}
		
		if( MousePos.y > Height/2) //Look Down
		{
			Angle.X -= updown_speed;
			
			if( Angle.X < -80.0f)
			{
				Angle.X = -80.0f;
			}
			//if( Angle.X < 0.0f )
			//	Angle.X += 360.0f;
		}
		
		reset_cursor();
		
		printExtended("Collision detection.\n");
		
		//-------------------
		//move
		//-------------------
		
		//change
		
		
		if( player_getState() == STATE_WATER )
		{

		}
		
		//Move forward
		if ( keys[ controls.forward ] )
		{
			geFloat multi = 1.0f;//sneak
			
			if( ! keys[controls.crouch] )
				moved = 1;
			
			if( keys[ controls.holdwalk ] )
			{
				multi = 5.0f;
			}
			else
			{
				multi = 10.0f;
			}
			geXForm3d_GetIn(&XForm, &in);
			/////////////////////////////////////////////////////////////////////////////////////////////////////
			if( player_getState() == STATE_NORMAL  && !cheats.ghost )
				in.Y = 0.0f; // will this remove the ugly thing in walking/jumping ???
			/////////////////////////////////////////////////////////////////////////////////////////////////////
			geVec3d_AddScaled(&CurrentSpeed, &in, speed * multi * TIME * heroTime, &CurrentSpeed);
		}
		
		//Move backward
		if(keys[ controls.backward ] )
		{
			geFloat multi = 1.0f;//sneak
			
			if( ! keys[controls.crouch] )
				moved = 1;
			
			if( keys[ controls.holdwalk ] )
			{
				multi = 5.0f;
			}
			else
			{
				multi = 10.0f;
			}
			
			geXForm3d_GetIn(&XForm, &in);
			if( player_getState() == STATE_NORMAL && !cheats.ghost)
				in.Y = 0.0f;
			geVec3d_AddScaled(&CurrentSpeed, &in, speed*multi*-1 * TIME * heroTime, &CurrentSpeed);
		}
		
		//Strafe left
		if(keys[ controls.left ] )
		{
			geFloat multi = 1.0f;//sneak
			
			if( ! keys[controls.crouch] )
				moved = 1;
			
			if( keys[ controls.holdwalk ] )
			{
				multi = 3.0f;
			}
			else
			{
				multi = 8.0f;
			}
			
			geXForm3d_GetLeft(&XForm, &left);
			left.Y = 0.0f;
			geVec3d_AddScaled(&CurrentSpeed, &left, speed*multi * TIME * heroTime, &CurrentSpeed);
		}
		
		//Strafe right
		if(keys[ controls.right ] )
		{
			geFloat multi = 1.0f;//sneak
			
			if( ! keys[controls.crouch] )
				moved = 1;
			
			if( keys[ controls.holdwalk ] )
			{
				multi = 3.0f;
			}
			else
			{
				multi = 8.0f;
			}
			
			geXForm3d_GetLeft(&XForm, &left);
			left.Y =0.0f;
			geVec3d_AddScaled(&CurrentSpeed, &left, speed*multi*-1 * TIME * heroTime, &CurrentSpeed);
		}
		
		if( keys[ controls.crouch ] )
		{
			if( player_getState() == STATE_WATER )
			{
				geXForm3d_GetUp(&XForm, &up);
				geVec3d_AddScaled(&CurrentSpeed, &up, speed * TIME * heroTime * -2, &CurrentSpeed);
				moved = 1;
			}
		}

		//Up
		if( keys[ controls.jump ] )
		{
			unsigned char state = player_getState();
			if( state != STATE_WATER )
			{
				if( dojump )
				{
					dojump = GE_FALSE;
					
					if( state == STATE_LADDER )
					{
						geFloat multi = 1.0f;//sneak
						if( keys[ controls.holdwalk ] )
						{
							multi = 3.0f;
						}
						else
						{
							multi = 8.0f;
						}
						///geVec3d_AddScaled(&Pos, &inLadder, speed*multi*-1 * TIME, &Pos);
					}
					else
					{
#ifdef MULTIPLE_JUMPS
						if( ySpeed < 300.0f && ySpeed > -100.0f && player_jumps<MULTIPLE_JUMPS){
							player_jumps++;
#else
 						if( player_is_on_ground() ){
#endif
							ySpeed = keys[ controls.crouch ] ? PLAYER_JUMP_SPEED*1.25f : PLAYER_JUMP_SPEED;
							ySpeed *= timedamage_has(TIMEDAMAGE_DRUG) ? 1.3f : 1.0f;
							//keys[ controls.jump ] = GE_FALSE;
							
							soundsys_play_sound( &jump[ rand()%3 ], GE_FALSE );
						}
					}
				}
			}
			else
			{
				geXForm3d_GetUp(&XForm, &up);
				geVec3d_AddScaled(&CurrentSpeed, &up, speed * TIME * heroTime, &CurrentSpeed);
				ySpeed = PLAYER_JUMP_SPEED;
				//moved = 1;
			}
		}
		else
		{
			dojump = GE_TRUE;
		}

		if( keys[ controls.toggleFlashlight ] )
		{
			if( !flashkeyDown )
			{
				flashkeyDown = GE_TRUE;
				flip_flashlight();
			}
		}
		else
		{
			flashkeyDown = GE_FALSE;
		}
		
		//Down
		
		if( keys[ controls.jet ] )
		{
			if( player_getState() == STATE_WATER )
			{
				//geXForm3d_GetUp(&XForm, &up);
				//geVec3d_AddScaled(&Pos, &up, speed*-1 * TIME , &Pos);
			}
			else
			{
				// duck
				ySpeed += (gravity+260.0f) * TIME * gravityTime;
			}
		}
		
		//--------------------------------------
		
		if( keys[ controls.reload ] )
		{
			weapon_reload();
			keys[ controls.reload ] = GE_FALSE;
		}

		if( keys[ controls.wc1 ] )
		{
			weapon_select(0);
			keys[controls.wc1] = GE_FALSE;
		}
		
		if( keys[ controls.wc2 ] )
		{
			weapon_select(1);
			keys[ controls.wc2 ] = GE_FALSE;
		}
		
		if( keys[ controls.wc3 ] )
		{
			weapon_select(2);
			keys[ controls.wc3 ] = GE_FALSE;
		}
		
		if( keys[ controls.wc4 ] )
		{
			weapon_select(3);
			keys[controls.wc4] = GE_FALSE;
		}
		
		if( keys[ controls.wc5 ] )
		{
			weapon_select(4);
			keys[controls.wc5] = GE_FALSE;
		}
		
		if( keys[controls.wc6] )
		{
			weapon_select(5);
			keys[controls.wc6] = GE_FALSE;
		}

		if( keys[ controls.use ] )
		{
			if( !usekeyDown )
			{
				usekeyDown = GE_TRUE;
				player_use();
			}
		}
		else
		{
			usekeyDown = GE_FALSE;
		}
		
		// primary weapon fire
		if( mouse[0] )
		{
			weapon_fire( mbnew[0] );
		}
		
		// secondary weapon fire
		if( mouse[2] )
		{
			weapon_sfire( mbnew[2] );
		}
		
		if( keys[ controls.previous ] )
		{
			weapon_previous();
			keys[ controls.previous ] = GE_FALSE;
		}
		
		if( keys[ controls.next ] )
		{
			weapon_next();
			keys[ controls.next ] = GE_FALSE;
		}

		// will no be a function in the final game. That's why this has a char, not a key.
		if( keys['T'] )
		{
			mugshot();
			keys['T'] = GE_FALSE;
		}

		if( keys[ controls.quickSave ] )
		{
			if( !qsaveDown )
			{
				qsaveDown = GE_TRUE;
				quick_save();
			}
		}
		else
		{
			qsaveDown = GE_FALSE;
		}
	}//end of player alive
	
	
		if( keys[ controls.quickLoad ] )
		{
			if( !qloadDown )
			{
				qloadDown = GE_TRUE;
				quick_load();
			}
		}
		else
		{
			qloadDown = GE_FALSE;
		}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////




void StopXZ()
{
	geVec3d prePos;
	geFloat slide;
	
	prePos = Pos;

	Pos.X += CurrentSpeed.X;
	Pos.Y += CurrentSpeed.Y;
	Pos.Z += CurrentSpeed.Z;
	
	if( !cheats.ghost && geWorld_Collision(
		World,					//The world
		&ExtBox.Min,			//Boundingbox min
		&ExtBox.Max,			//Boundingbox max
		&prePos,				//Old pos
		&Pos,					//Future pos
		GE_CONTENTS_SOLID_CLIP,	//Collission type
		GE_COLLIDE_ALL,			//Collission type
		0xffffffff,				//?
		cb_move,				//?
		NULL,					//?
		&Col					//We need to save the collission info data
		)//end of function call
		)//end of if
	{
		front.X = CurrentSpeed.X;
		front.Y = 0;
		front.Z = CurrentSpeed.Z;
		
		if( StepUp(&prePos) )
		{
			
			slide = geVec3d_DotProduct (&Pos, &Col.Plane.Normal) - Col.Plane.Dist;
			//re-position the player
			Pos.X -= Col.Plane.Normal.X * slide;
			Pos.Y -= Col.Plane.Normal.Y * slide;
			Pos.Z -= Col.Plane.Normal.Z * slide;
			
			//did we step through a crack in the wall
			if	(geWorld_Collision(
				World,					//The world
				&ExtBox.Min,			//Boundingbox min
				&ExtBox.Max,			//Boundingbox max
				&prePos,				//Old pos
				&Pos,					//Future pos
				GE_CONTENTS_SOLID_CLIP,	//Collission type
				GE_COLLIDE_ALL,			//Collission type
				0xffffffff,				//?
				cb_move,				//?
				NULL,					//?
				&Col					//We need to save the collission info data
				)//end of function call
				)//end of if
			{	
				{
					slide = geVec3d_DotProduct (&Pos, &Col.Plane.Normal) - Col.Plane.Dist;
					//re-position the player
					Pos.X -= Col.Plane.Normal.X * slide;
					Pos.Y -= Col.Plane.Normal.Y * slide;
					Pos.Z -= Col.Plane.Normal.Z * slide;
					
					//did we step through a crack in the wall
					if	(geWorld_Collision(
						World,					//The world
						&ExtBox.Min,			//Boundingbox min
						&ExtBox.Max,			//Boundingbox max
						&prePos,				//Old pos
						&Pos,					//Future pos
						GE_CONTENTS_SOLID_CLIP,	//Collission type
						GE_COLLIDE_ALL,			//Collission type
						0xffffffff,				//?
						cb_move,				//?
						NULL,					//?
						&Col					//We need to save the collission info data
						)//end of function call
						)//end of if
					{
						slide = geVec3d_DotProduct (&Pos, &Col.Plane.Normal) - Col.Plane.Dist;
						//re-position the player
						Pos.X -= Col.Plane.Normal.X * slide;
						Pos.Y -= Col.Plane.Normal.Y * slide;
						Pos.Z -= Col.Plane.Normal.Z * slide;
						
						//did we step through a crack in the wall
						if	(geWorld_Collision(
							World,					//The world
							&ExtBox.Min,			//Boundingbox min
							&ExtBox.Max,			//Boundingbox max
							&prePos,				//Old pos
							&Pos,					//Future pos
							GE_CONTENTS_SOLID_CLIP,	//Collission type
							GE_COLLIDE_ALL,			//Collission type
							0xffffffff,				//?
							cb_move,				//?
							NULL,					//?
							&Col					//We need to save the collission info data
							)//end of function call
							)//end of if
						{
							Pos = Col.Impact;
						}
						//sprintf(str, "NewPos: (%f, %f, %f)!\n", Pos.X, Pos.Y, Pos.Z);
						//printLog(str);
					}
				}
			}
		}
		else
		{
			Pos = prePos;
		}
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//		apply_gravity()
//			takes care of jump/fall colliding and damaging
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void apply_gravity()
{
	geVec3d prePos;
	geFloat slide;
	geBoolean preLanded = landed;
	geFloat sq_speed;
	char do_bounce = 0;
	
	prePos = Pos;
	
	if( player_getState() == STATE_LADDER || cheats.ghost )
	{
		StopXZ();
		ySpeed = 0.0f;
		return;
	}
	else
	{
		//update y speed
		if( player_getState() == STATE_WATER )
		{
			if( moved )
			{
				ySpeed = 0;
			}
			else
			{
				ySpeed += gravity * 0.04f * TIME * gravityTime;
			}
		}
		else
		{
			ySpeed -= gravity * TIME * gravityTime;
		}
	}

	StopXZ();

	//move up/down
	Pos.Y += ySpeed*TIME * gravityTime;

	if( ySpeed > MAX_Y_SPEED )		ySpeed = MAX_Y_SPEED;
	if( ySpeed < -MAX_Y_SPEED )		ySpeed = -MAX_Y_SPEED;
	
	//if we have landed, we have not landed again - so set it to false
	landed = GE_FALSE;
	
	//check collision
	if( !cheats.ghost && geWorld_Collision(
		World,					//The world
		&ExtBox.Min,			//Boundingbox min
		&ExtBox.Max,			//Boundingbox max
		&prePos,				//Old pos
		&Pos,					//Future pos
		GE_CONTENTS_SOLID_CLIP,	//Collission type
		GE_COLLIDE_ALL,			//Collission type
		0xffffffff,				//?
		cb_move,				//?
		NULL,					//?
		&Col					//We need to save the collission info data
		)//end of function call
		)//end of if
	{
		{
			//we have been falling and we have hit the floor
			landed = GE_TRUE;
			if( landed != preLanded )
			{
				if( ySpeed < -350.0f )
					do_bounce = 1;

				if( do_bounce || cheats.bouncy ) // Some minor speed optimation
				{
					if (geWorld_Collision(World, &ExtBox.Min, &ExtBox.Max, &prePos,&Pos,
						 CONTENTS_BOUNCY, GE_COLLIDE_ALL, 0,
						 NULL, NULL, &Col) || cheats.bouncy 
					   )
					{
						//do_bounce = 1; // No need to set it' since it's already set
						soundsys_play_sound( &boing, GE_FALSE );
					}
					else
					{
						do_bounce = 0;
					}
				}

				if( !do_bounce )
				{
					sq_speed = FLOAT_ABS(ySpeed);
					/*
					sprintf(str, "Speed: %f!", sq_speed);
					system_message(str);
					*/
					
					if(sq_speed < 350.0f )
					{
						//nothing
					}else if(sq_speed < 1000.0f )
					{
						//sound
						if( ySpeed < 0.0f )
							soundsys_play_sound(&land_normal[rand()%3], GE_FALSE);
					}else if(sq_speed < 1600.0f )
					{
						//damage 25
						damage(5 + rand()%15);
						game_message("Take it easy sports!");
					}else if(sq_speed < 16000.0f )
					{
						//damage 50
						game_message("Ouch, that hurt!");
						armor_piercing_damage(15 + rand()%45 );
					}else if(sq_speed < 19000.0f )
					{
						//damage 75
						game_message("I broke a bone!");
						armor_piercing_damage(45 + rand()%105);
					}else if(sq_speed < 25000.0f )
					{
						//damage 100
						game_message("I broke my neck!");
						armor_piercing_damage(105);
					}else
					{
						game_message("I'm a giant pizza!"); //red dwarf - episode 1
						armor_piercing_damage(254);
					}
				}
			}
		}
		
		if( do_bounce )
		{
			ySpeed *= (keys[ controls.jump ] ) ? -1.1f : -0.7f;
		}
		else
		{
			ySpeed = 0.0f;
		}

#ifdef MULTIPLE_JUMPS
		player_jumps = 0;
#endif
		
		//slide
		
		//calculate slide
		slide = geVec3d_DotProduct (&Pos, &Col.Plane.Normal) - Col.Plane.Dist;
		//re-position the player
		Pos.X -= Col.Plane.Normal.X * slide;
		Pos.Y -= Col.Plane.Normal.Y * slide;
		Pos.Z -= Col.Plane.Normal.Z * slide;
		
		//did we step through a crack in the wall
		if	(geWorld_Collision(
			World,					//The world
			&ExtBox.Min,			//Boundingbox min
			&ExtBox.Max,			//Boundingbox max
			&prePos,				//Old pos
			&Pos,					//Future pos
			GE_CONTENTS_SOLID_CLIP,	//Collission type
			GE_COLLIDE_ALL,			//Collission type
			0xffffffff,				//?
			cb_move,				//?
			NULL,					//?
			&Col					//We need to save the collission info data
			)//end of function call
			)//end of if
		{
			Pos = Col.Impact;
			//sprintf(str, "NewPos: (%f, %f, %f)!\n", Pos.X, Pos.Y, Pos.Z);
			//printLog(str);
		}
	}
	else
	{
		//free falling, or free jumping
	}
}

void set_speed()
{
	// when the code piece below is uncommented it gives the player 
	// the ability to hold crouch down in air and not loose any speed
	// player might crouch through the entire game and jump very high
	// all the time, and this is definitely not wanted. Therefore I've
	// disabled it
	speed = (keys[controls.crouch] && player_is_on_ground() )? real_speed / 3.5f : real_speed;
	//speed = (keys[controls.crouch] /*&& player_is_on_ground()*/ )? real_speed / 3.5f : real_speed;
}

/* soundsys_sound bob_normal_right[5];
soundsys_sound bob_normal_left[5];

soundsys_sound bob_water_right[5];
soundsys_sound bob_water_left[5]; */

void play_walksound_right()
{
	if( player_getState() == STATE_WATER )
	{
		soundsys_play_sound_ex(&bob_water_right[generate_random(5)], GE_FALSE, TYPE_RIGHT);
	}
	else if( player_getState() == STATE_LADDER )
	{
		soundsys_play_sound_ex(&bob_ladder_right[ rand() % 5 ], GE_FALSE, TYPE_RIGHT);
	}
	else
	{
		if( player_is_on_ground() )
			soundsys_play_sound_ex(&bob_normal_right[generate_random(5)], GE_FALSE, TYPE_RIGHT);
	}
}

void play_walksound_left()
{
	if( player_getState() == STATE_WATER )
	{
		soundsys_play_sound_ex(&bob_water_left[generate_random(5)], GE_FALSE, TYPE_LEFT);
	}
	else if( player_getState() == STATE_LADDER )
	{
		soundsys_play_sound_ex(&bob_ladder_left[ rand() % 5 ], GE_FALSE, TYPE_LEFT);
	}
	else
	{
		if( player_is_on_ground() )
			soundsys_play_sound_ex(&bob_normal_left[generate_random(5)], GE_FALSE, TYPE_LEFT);
	}
}

void enter_water()
{
	ySpeed /= 10.0f;
	if( !options.fogInWater ) return;
	set_fog(0.0f, 30.0f, 40.0f, options.meterfog, 1);
}

void leave_water()
{
	if( !options.fogInWater ) return;
	level_enableLevelFog();
}
