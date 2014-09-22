/******************************************************************************/
/*  EffManager.c                                                              */
/*                                                                            */
/*  Author: David Wulff, Ralph Deane                                          */
/*  Description: Effect manager                                               */
/*                                                                            */
/*  Copyright (c) 1999, 2000, Battleaxe Studios; All rights reserved.         */
/*           ______  __   __  ___      __  __  __  __  __  _____    __        */
/*          / ____/ / /  / / / _ \    / / / / / / / / / / / ___ \  / /        */
/*         / /___  / /  / / / /_\ \  / / / / / / / / / / / /__/ / / /         */
/*        /___  / / /  / / / ___  / / / / / / / / / / / /  __  / / /          */
/*       ____/ / / /__/ / / /  / /  \ \/ / / /  \ \/ / / /  / / / /___        */
/*      /_____/ /______/ /_/  /_/    \__/ /_/    \__/ /_/  /_/ /_____/        */
/*                                                                            */
/*                              www.bttlxe.co.uk                              */
/******************************************************************************/

//#include "GMain.h"

#include "EffParticle.h"
#include "EffManager.h"
#include "Ram.h"
#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include <math.h>
#include <assert.h>

#include "useful_macros.h"

static float EffectC_Frand(float Low, float High );
static void EffectC_XFormFromVector(geVec3d *Source, geVec3d *Target, geXForm3d *Out );
static geBoolean EffectC_IsPointVisible(geWorld *World, geCamera *Camera, geVec3d *Target, int32 Leaf,	uint32 ClipStyle );

//
// Effect Manager Constructor
//

Eff_Manager *EM_Create(geEngine *Engine, geCamera *Camera, geWorld *World)
{
	Eff_Manager *EM = malloc(sizeof(Eff_Manager)); // create the memory for the Eff_Manager
	int i;

	if( !EM ) return 0;
	
	EM->Engine = Engine; // save the engine for use by the effects
	EM->Camera = Camera;
	EM->World = World;
	
	TPool_Initalize(); // initalize the texture manager
	SPool_Initalize(); // initalize the sound manager
	
	// clear out all items
	for (i=0;i<MAX_EFF_ITEMS;i++)
	{
		EM->Item[i].Active=GE_FALSE;
		EM->Item[i].Pause=GE_FALSE;
	}
	
	// create the particle system handler
	EM->Ps = Particle_SystemCreate(EM->World);
	
	EM->m_LastTick = timeGetTime();
	
	return EM;
}

//
// Effect Manager Destructor
//

EM_Destroy(Eff_Manager *EM)
{
	int i;

	for (i=0;i<MAX_EFF_ITEMS;i++)
	{
		// do only if item is currently active
		if(EM->Item[i].Active==GE_TRUE)
		{
			// have each effect type remove any data
			switch(EM->Item[i].Type)
			{
			case EFF_LIGHT:
				EM_Glow_Remove(EM, (Glow *)EM->Item[i].Data);
				EM->Item[i].Active=GE_FALSE;
				EM->Item[i].Pause=GE_FALSE;
				break;
			case EFF_SPRAY:
				EM_Spray_Remove(EM, (Spray *)EM->Item[i].Data);
				EM->Item[i].Active=GE_FALSE;
				EM->Item[i].Pause=GE_FALSE;
				break;
			case EFF_SPRITE:
				EM_Sprite_Remove(EM, (Sprite *)EM->Item[i].Data);
				EM->Item[i].Active=GE_FALSE;
				EM->Item[i].Pause=GE_FALSE;
				break;
			case EFF_SND:
				EM_Snd_Remove(EM, (Snd *)EM->Item[i].Data);
				EM->Item[i].Active=GE_FALSE;
				EM->Item[i].Pause=GE_FALSE;
				break;
			case EFF_BOLT:
				EM_Bolt_Remove(EM, (Bolt *)EM->Item[i].Data);
				EM->Item[i].Active=GE_FALSE;
				EM->Item[i].Pause=GE_FALSE;
				break;
			case EFF_ELECTRICBOLT:
				EM_EBolt_Remove(EM, (ElectricBolt*)EM->Item[i].Data);
				EM->Item[i].Active=GE_FALSE;
				EM->Item[i].Pause=GE_FALSE;
				break;
			case EFF_CORONA:
				EM_Corona_Remove(EM, (EffCorona *)EM->Item[i].Data);
				EM->Item[i].Active=GE_FALSE;
				EM->Item[i].Pause=GE_FALSE;
				break;
			case EFF_BLOOD:
				EM_Blood_Remove(EM, (Blood *)EM->Item[i].Data);
				EM->Item[i].Active=GE_FALSE;
				EM->Item[i].Pause=GE_FALSE;
				break;
			default:
				break;
			}
		}
	}
	
	
	// remove the particle system handler
	Particle_SystemDestroy(EM->Ps);
	TPool_Delete(EM->World); // delete the textures
	SPool_Delete(EM->AudioSystem); // delete the sounds
}

//
// Effect Manager Tick  - Process all effects
//

geBoolean EM_Tick(Eff_Manager *EM, float dwTicks)
{
	int i;
	
	//float dwTicks;
	// get time in seconds since last pass
	//dwTicks = (float)(timeGetTime() - EM->m_LastTick)*0.001f;
	
	for (i=0;i<MAX_EFF_ITEMS;i++)
	{
		// process if active and not paused
		if(EM->Item[i].Active==GE_TRUE && EM->Item[i].Pause==GE_FALSE)
		{
			// if effect process returns false then remove it
			switch(EM->Item[i].Type)
			{
			case EFF_LIGHT:
				if(EM_Glow_Process(EM, (Glow  *)EM->Item[i].Data,  dwTicks)==GE_FALSE)
				{
					EM_Glow_Remove(EM, (Glow *)EM->Item[i].Data);
					EM->Item[i].Active=GE_FALSE;
				}
				break;
			case EFF_SPRAY:
				if(EM_Spray_Process(EM, (Spray  *)EM->Item[i].Data,  dwTicks)==GE_FALSE)
				{
					EM_Spray_Remove(EM, (Spray *)EM->Item[i].Data);
					EM->Item[i].Active=GE_FALSE;
				}
				break;
			case EFF_SPRITE:
				if(EM_Sprite_Process(EM, (Sprite  *)EM->Item[i].Data,  dwTicks)==GE_FALSE)
				{
					EM_Sprite_Remove(EM, (Sprite *)EM->Item[i].Data);
					EM->Item[i].Active=GE_FALSE;
				}
				break;
			case EFF_SND:
				if(EM_Snd_Process(EM, (Snd  *)EM->Item[i].Data,  dwTicks)==GE_FALSE)
				{
					EM_Snd_Remove(EM, (Snd *)EM->Item[i].Data);
					EM->Item[i].Active=GE_FALSE;
				}
				break;
			case EFF_BOLT:
				if(EM_Bolt_Process(EM, (Bolt  *)EM->Item[i].Data,  dwTicks)==GE_FALSE)
				{
					EM_Bolt_Remove(EM, (Bolt *)EM->Item[i].Data);
					EM->Item[i].Active=GE_FALSE;
				}
				break;
			case EFF_ELECTRICBOLT:
				if(EM_EBolt_Process(EM, (ElectricBolt*)EM->Item[i].Data,  dwTicks)==GE_FALSE)
				{
					EM_EBolt_Remove(EM, (ElectricBolt*)EM->Item[i].Data);
					EM->Item[i].Active=GE_FALSE;
				}
				break;
			case EFF_CORONA:
				if(EM_Corona_Process(EM, (EffCorona  *)EM->Item[i].Data,  dwTicks)==GE_FALSE)
				{
					EM_Corona_Remove(EM, (EffCorona *)EM->Item[i].Data);
					EM->Item[i].Active=GE_FALSE;
				}
				break;
			case EFF_BLOOD:
				if(EM_Blood_Process(EM, (Blood  *)EM->Item[i].Data,  dwTicks)==GE_FALSE)
				{
					EM_Blood_Remove(EM, (Blood *)EM->Item[i].Data);
					EM->Item[i].Active=GE_FALSE;
				}
				break;
			default:
				break;
			}
		}

	}

	// process the particles
	Particle_SystemFrame(EM->Ps, dwTicks);

	//EM->m_LastTick = timeGetTime();
	
	return GE_TRUE;
}

//
// Effect Manager Add new Effect item
//     - Itype is effect type
//     - Idata is the effect data
//

int EM_Item_Add(Eff_Manager *EM, int Itype, void *Idata)
{
	int i;
	
	for (i=0;i<MAX_EFF_ITEMS;i++)
	{
		// find a empty slot to store it in
		if(EM->Item[i].Active==GE_FALSE)
		{
			// save effect type
			EM->Item[i].Type=Itype;
			// process and save the data, set item active
			// returns the slot index used so that the effect
			// can be modified later
			switch(Itype)
			{
			case EFF_LIGHT:
				EM->Item[i].Data=EM_Glow_Add(EM, Idata);
				EM->Item[i].Active=GE_TRUE;
				return i;
				break;
			case EFF_SPRAY:
				EM->Item[i].Data=EM_Spray_Add(EM, Idata);
				EM->Item[i].Active=GE_TRUE;
				return i;
				break;
			case EFF_SPRITE:
				EM->Item[i].Data=EM_Sprite_Add(EM, Idata);
				EM->Item[i].Active=GE_TRUE;
				return i;
				break;
			case EFF_SND:
				EM->Item[i].Data=EM_Snd_Add(EM, Idata);
				EM->Item[i].Active=GE_TRUE;
				return i;
				break;
			case EFF_BOLT:
				EM->Item[i].Data=EM_Bolt_Add(EM, Idata);
				EM->Item[i].Active=GE_TRUE;
				return i;
				break;
			case EFF_ELECTRICBOLT:
				EM->Item[i].Data=EM_EBolt_Add(EM, Idata);
				EM->Item[i].Active=GE_TRUE;
				return i;
				break;
			case EFF_CORONA:
				EM->Item[i].Data=EM_Corona_Add(EM, Idata);
				EM->Item[i].Active=GE_TRUE;
				return i;
				break;
			case EFF_BLOOD:
				EM->Item[i].Data=EM_Blood_Add(EM, Idata);
				EM->Item[i].Active=GE_TRUE;
				return i;
				break;
			default:
				break;
			}
		}
	}
	// index of -1 is failure
	return -1;
}

//
// Effect Manager Modify effect item
//     - Itype is effect type
//     - Index is slot index
//     - Data is new effect data
//     - Flags indicate which parts of the data is used
//           to update effect data
//

void EM_Item_Modify(Eff_Manager *EM, int Itype, int Index, void *Data, uint32 Flags)
{
    // make sure that item types match and that it is active
    if(EM->Item[Index].Type==Itype && EM->Item[Index].Active==GE_TRUE)
    {
		// modify effect data
		switch(Itype)
		{
        case EFF_LIGHT:
			EM_Glow_Modify(EM, (Glow *)EM->Item[Index].Data, (Glow *)Data, Flags);
			break;
        case EFF_SPRAY:
			EM_Spray_Modify(EM, (Spray *)EM->Item[Index].Data, (Spray *)Data, Flags);
			break;
        case EFF_SPRITE:
			EM_Sprite_Modify(EM, (Sprite *)EM->Item[Index].Data, (Sprite *)Data, Flags);
			break;
        case EFF_SND:
			EM_Snd_Modify(EM, (Snd *)EM->Item[Index].Data, (Snd *)Data, Flags);
			break;
		case EFF_BOLT:
			EM_Bolt_Modify(EM, (Bolt *)EM->Item[Index].Data, (Bolt *)Data, Flags);
			break;
		case EFF_CORONA:
			EM_Corona_Modify(EM, (EffCorona *)EM->Item[Index].Data, (EffCorona *)Data, Flags);
			break;
        default:
			break;
		}
    }
}

//
// Effect Manager Delete effect item
//

void EM_Item_Delete(Eff_Manager *EM, int Itype, int Index)
{
    // delete only if types match and is active
    if(EM->Item[Index].Type==Itype && EM->Item[Index].Active==GE_TRUE)
    {
		// remove the effects data
		switch(Itype)
		{
        case EFF_LIGHT:
			EM_Glow_Remove(EM, (Glow *)EM->Item[Index].Data);
			EM->Item[Index].Active=GE_FALSE;
			break;
        case EFF_SPRAY:
			EM_Spray_Remove(EM, (Spray *)EM->Item[Index].Data);
			EM->Item[Index].Active=GE_FALSE;
			break;
        case EFF_SPRITE:
			EM_Sprite_Remove(EM, (Sprite *)EM->Item[Index].Data);
			EM->Item[Index].Active=GE_FALSE;
			break;
        case EFF_SND:
			EM_Snd_Remove(EM, (Snd *)EM->Item[Index].Data);
			EM->Item[Index].Active=GE_FALSE;
			break;
		case EFF_BOLT:
			EM_Bolt_Remove(EM, (Bolt *)EM->Item[Index].Data);
			EM->Item[Index].Active=GE_FALSE;
			break;
		case EFF_ELECTRICBOLT:
			EM_EBolt_Remove(EM, (ElectricBolt *)EM->Item[Index].Data);
			EM->Item[Index].Active=GE_FALSE;
			break;
		case EFF_CORONA:
			EM_Corona_Remove(EM, (EffCorona *)EM->Item[Index].Data);
			EM->Item[Index].Active=GE_FALSE;
			break;
		case EFF_BLOOD:
			EM_Blood_Remove(EM, (Blood *)EM->Item[Index].Data);
			EM->Item[Index].Active=GE_FALSE;
			break;
        default:
			break;
		}
    }
}

//
// Effect Manager Pause/Unpause an effect
//     - Flag is state of pause flag
//

void EM_Item_Pause(Eff_Manager *EM, int Itype, int Index, geBoolean Flag)
{
    // change only if types match and is active
    if(EM->Item[Index].Type==Itype && EM->Item[Index].Active==GE_TRUE)
		EM->Item[Index].Pause=Flag;
    // do processing for bolt if it is paused
    if(Itype==EFF_BOLT)
		EM_Bolt_Pause(EM, (Bolt *)EM->Item[Index].Data, Flag);
}

//---------------------------------------------------------------------
//   Private Functions to handle each effect
//
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// Spray
//---------------------------------------------------------------------
void *EM_Spray_Add(Eff_Manager *EM, void *Data)
{
	Spray	*NewData;
	
	NewData = ALLOCATE_STRUCT(Spray);
	if ( NewData == NULL )
		return NULL;
	
	memcpy( NewData, Data, sizeof( *NewData ) );
	
	// setup defaults
	NewData->TimeRemaining = 0.0f;
	NewData->PolyCount = 0.0f;
	
	// setup particle gravity
	if ( geVec3d_Length( &( NewData->Gravity ) ) > 0.0f )
	{
		NewData->ParticleGravity = &( NewData->Gravity );
	}
	else
	{
		NewData->ParticleGravity = (geVec3d *)NULL;
	}
	
	// save the transform
	EffectC_XFormFromVector( &( NewData->Source ), &( NewData->Dest ), &( NewData->Xf ) );
	
	// setup default vertex data
	NewData->Vertex.u = 0.0f;
	NewData->Vertex.v = 0.0f;
	NewData->Vertex.r = 255.0f;
	NewData->Vertex.g = 255.0f;
	NewData->Vertex.b = 255.0f;
	
	// calculate leaf value
	geWorld_GetLeaf( EM->World, &( NewData->Source ), &( NewData->Leaf ) );
	
	return (void *)NewData;
}

void EM_Spray_Remove(Eff_Manager *EM, Spray *Data)
{
	if (Data->AnchorPoint != NULL )
		Particle_SystemRemoveAnchorPoint( EM->Ps, Data->AnchorPoint );
	
	// free effect data
	free( Data );
	
}

geBoolean EM_Spray_Process(Eff_Manager *EM, Spray  *Data,  float  TimeDelta)
{
	
	// locals
	geVec3d			Velocity;
	geVec3d			Left, Up;
	geVec3d			Source, Dest;
	const geXForm3d		*CameraXf;
	float			ScaleFrom, ScaleTo;
	float			UnitLife;
	float			Distance;
	float			Adjustment = 1.0f;
	float			NewPolyCount = 0.0f;

	if( Data->waitTime > 0.0f ){
		Data->waitTime -= TimeDelta;
		return GE_TRUE;
	}
	
	// adjust spray life, killing it if required
	if ( Data->SprayLife > 0.0f )
	{
		Data->SprayLife -= TimeDelta;
		if ( Data->SprayLife <= 0.0f )
		{
			return GE_FALSE;
		}
	}
	
	// do nothing if its paused
	if ( Data->Paused == GE_TRUE )
	{
		return GE_TRUE;
	}
	
	// do nothing if it isn't visible
	if(Data->ShowAlways==GE_FALSE)
	{
		if ( EffectC_IsPointVisible(EM->World,
			EM->Camera,
			&( Data->Source ),
			Data->Leaf,
			EFFECTC_CLIP_LEAF | EFFECTC_CLIP_SEMICIRCLE ) == GE_FALSE )
			return GE_TRUE;
	}
	
	// get camera xform
	CameraXf = geCamera_GetWorldSpaceXForm( EM->Camera );
	
	// perform level of detail processing if required
	if ( Data->DistanceMax > 0.0f )
	{
		
		// do nothing if its too far away
		Distance = geVec3d_DistanceBetween( &( Data->Source ), &( CameraXf->Translation ) );
		if ( Distance > Data->DistanceMax )
		{
			return GE_TRUE;
		}
		
		// determine polygon adjustment amount
		if ( ( Data->DistanceMin > 0.0f ) && ( Distance > Data->DistanceMin ) )
		{
			Adjustment = ( 1.0f - ( ( Distance - Data->DistanceMin ) / ( Data->DistanceMax - Data->DistanceMin ) ) );
		}
	}
	
	// determine how many polys need to be added taking level fo detail into account
	Data->TimeRemaining += TimeDelta;
	while ( Data->TimeRemaining >= Data->Rate )
	{
		Data->TimeRemaining -= Data->Rate;
		NewPolyCount += 1.0f;
	}
	NewPolyCount *= Adjustment;
	Data->PolyCount += NewPolyCount;
	
	// add new textures
	while ( Data->PolyCount > 0 )
	{
		
		// adjust poly remaining count
		Data->PolyCount -= 1.0f;
		
		// pick a source point
		if ( Data->SourceVariance > 0 )
		{
			geXForm3d_GetLeft( &( Data->Xf ), &Left );
			geXForm3d_GetUp( &( Data->Xf ), &Up );
			geVec3d_Scale( &Left, (float)Data->SourceVariance * EffectC_Frand( -1.0f, 1.0f ), &Left );
			geVec3d_Scale( &Up, (float)Data->SourceVariance * EffectC_Frand( -1.0f, 1.0f ), &Up );
			geVec3d_Add( &Left, &Up, &Source );
			geVec3d_Add( &( Data->Source ), &Source, &Source );
		}
		else
		{
			geVec3d_Copy( &( Data->Source ), &Source );
		}
		Data->Vertex.X = Source.X;
		Data->Vertex.Y = Source.Y;
		Data->Vertex.Z = Source.Z;
		
		// pick a destination point
		if ( Data->DestVariance > 0 )
		{
			geXForm3d_GetLeft( &( Data->Xf ), &Left );
			geXForm3d_GetUp( &( Data->Xf ), &Up );
			geVec3d_Scale( &Left, (float)Data->DestVariance * EffectC_Frand( -1.0f, 1.0f ), &Left );
			geVec3d_Scale( &Up, (float)Data->DestVariance * EffectC_Frand( -1.0f, 1.0f ), &Up );
			geVec3d_Add( &Left, &Up, &Dest );
			geVec3d_Add( &( Data->Dest ), &Dest, &Dest );
		}
		else
		{
			geVec3d_Copy( &( Data->Dest ), &Dest );
		}
		
		// set velocity
		if ( Data->MinSpeed > 0.0f )
		{
			geVec3d_Subtract( &Dest, &Source, &Velocity );
			geVec3d_Normalize( &Velocity );
			geVec3d_Scale( &Velocity, EffectC_Frand( Data->MinSpeed, Data->MaxSpeed ), &Velocity );
		}
		else
		{
			geVec3d_Set( &Velocity, 0.0f, 0.0f, 0.0f );
		}
		
		// set scale
		ScaleFrom = EffectC_Frand( Data->MinScale, Data->MaxScale );

		if( Data->ScaleStyle == SSS_RANDOM_SCALE ){
			ScaleTo = ScaleFrom;
		} else {
			ScaleFrom = Data->MinScale;
			ScaleTo = Data->MaxScale;
		}
		
		// set life
		UnitLife = EffectC_Frand( Data->MinUnitLife, Data->MaxUnitLife );
		
		// setup color
		Data->Vertex.r = EffectC_Frand( Data->ColorMin.r, Data->ColorMax.r );
		Data->Vertex.g = EffectC_Frand( Data->ColorMin.g, Data->ColorMax.g );
		Data->Vertex.b = EffectC_Frand( Data->ColorMin.b, Data->ColorMax.b );
		Data->Vertex.a = EffectC_Frand( Data->ColorMin.a, Data->ColorMax.a );
		
		// add the new particle
		if( Data->ScaleStyle == SSS_RANDOM_SCALE )
			Particle_SystemAddParticle(EM->Ps,
				Data->Texture,
				&( Data->Vertex ),
				Data->AnchorPoint,
				UnitLife,
				&Velocity,
				ScaleFrom, 0.0f, GE_FALSE,
				Data->ParticleGravity, 0, 0 );
		else
			Particle_SystemAddParticle(EM->Ps,
				Data->Texture,
				&( Data->Vertex ),
				Data->AnchorPoint,
				UnitLife,
				&Velocity,
				ScaleFrom, ScaleTo, GE_TRUE,
				Data->ParticleGravity, 0, 0 );
	}
	
	// all done
	return GE_TRUE;
	
} // EM_Spray_Process()

geBoolean EM_ParticleExplosion(Eff_Manager* EM, geVec3d location, geVec3d direction, geBitmap* image, float speed, geVec3d gravity, float change, float speedMultiply, int count, float life, float scale){
	GE_LVertex vertex;
	geVec3d velocity;
	int index;
	geVec3d directionChange;

	//geVec3d_AddScaled(&location, &direction, 0.01f, &location);

	// setup default vertex data
	vertex.u = 0.0f;
	vertex.v = 0.0f;
	vertex.r = 255.0f;
	vertex.g = 255.0f;
	vertex.b = 255.0f;
	vertex.a = 255.0f;

	vertex.X = location.X;
	vertex.Y = location.Y;
	vertex.Z = location.Z;

	for( index=0; index < count; index++){
		geVec3d_Set(&directionChange, EffectC_Frand(-1.0f * change, change), EffectC_Frand(-1.0f * change, change), EffectC_Frand(-1.0f * change, change));
		geVec3d_AddScaled(&direction, &directionChange, speed * EffectC_Frand(0.0f, speedMultiply), &velocity );

		Particle_SystemAddParticle(EM->Ps,
				image,
				&vertex,
				0,
				life,
				&velocity,
				scale, 0.0f, GE_FALSE,
				&gravity, 0, 0 );
	}

	return GE_TRUE;
}

geBoolean EM_Spray_Modify(Eff_Manager *EM, Spray *Data, Spray *NewData, uint32 Flags)
{
	geBoolean RecalculateLeaf = GE_FALSE;
	
	// adjust source and dest together
	if ( Flags & SPRAY_FOLLOWTAIL )
	{
		geVec3d_Copy( &( Data->Source ), &( Data->Dest ) );
		geVec3d_Copy( &( NewData->Source ), &( Data->Source ) );
		RecalculateLeaf = GE_TRUE;
	}
	
	// adjust source
	if ( Flags & SPRAY_SOURCE )
	{
		geVec3d_Copy( &( NewData->Source ), &( Data->Source ) );
		RecalculateLeaf = GE_TRUE;
	}
	
	// adjust source
	if ( Flags & SPRAY_DEST )
	{
		geVec3d_Copy( &( NewData->Dest ), &( Data->Dest ) );
		RecalculateLeaf = GE_TRUE;
	}
	
	// calculate leaf value
	if ( RecalculateLeaf == GE_TRUE )
	{
		EffectC_XFormFromVector( &( Data->Source ), &( Data->Dest ), &( Data->Xf ) );
		geWorld_GetLeaf(EM->World, &( Data->Source ), &( Data->Leaf ) );
	}
	
	// all done
	return GE_TRUE;
}


//---------------------------------------------------------------------
// Glow
//---------------------------------------------------------------------
void *EM_Glow_Add(Eff_Manager *EM, void *Data)
{
	Glow	*NewData;
	
	NewData = ALLOCATE_STRUCT(Glow);
	if ( NewData == NULL )
		return NULL;
	
	memcpy( NewData, Data, sizeof( *NewData ) );
	
	if ( NewData->RadiusMin < 0.0f )
		NewData->RadiusMin = 0.0f;
	if(NewData->RadiusMax <= 0.0f )
		NewData->RadiusMax = 0.1f;
	if( NewData->RadiusMax < NewData->RadiusMin )
		NewData->RadiusMax = NewData->RadiusMin;
	if (NewData->Intensity < 0.0f )
		NewData->Intensity = 0.0f;
	if (NewData->Intensity > 1.0f )
		NewData->Intensity = 1.0f;
	
	geWorld_GetLeaf(EM->World, &( NewData->Pos ), &( NewData->Leaf ) );
	
	return (void *)NewData;
}

void EM_Glow_Remove(Eff_Manager *EM, Glow *Data)
{
	if ( Data->Light != NULL )
	{
		geWorld_RemoveLight(EM->World, Data->Light );
		Data->Light = (geLight *)NULL;
	}
	free( Data );
}

geBoolean EM_Glow_Process(Eff_Manager *EM, Glow  *Data,  float  TimeDelta)
{
	float	Radius;
	GE_RGBA Color;
	
	
	// do nothing if it isn't visible
	if ( Data->DoNotClip == GE_FALSE )
	{
		if ( EffectC_IsPointVisible(EM->World,
			EM->Camera,
			&( Data->Pos ),
			Data->Leaf,
			EFFECTC_CLIP_LEAF | EFFECTC_CLIP_LINEOFSIGHT | EFFECTC_CLIP_SEMICIRCLE ) == GE_FALSE )
		{
			if ( Data->Light != NULL )
			{
				geWorld_RemoveLight( EM->World, Data->Light );
				Data->Light = (geLight *)NULL;
			}
			return GE_TRUE;
		}
	}
	
	// recreate the light if required
	if ( Data->Light == NULL )
	{
		Data->Light = geWorld_AddLight( EM->World );
		if ( Data->Light == NULL )
			return GE_TRUE;
		
	}
	
	// set color
	if( Data->ColorMin.r < -255.0f )
		Data->ColorMin.r = -255.0f;
	if( Data->ColorMax.r > 255.0f )
		Data->ColorMax.r = 255.0f;
	if( Data->ColorMin.r > Data->ColorMax.r )
		Data->ColorMin.r = Data->ColorMax.r;
	Color.r = EffectC_Frand( Data->ColorMin.r, Data->ColorMax.r ) * Data->Intensity;
	if( Data->ColorMin.g < -255.0f )
		Data->ColorMin.g = -255.0f;
	if( Data->ColorMax.g > 255.0f )
		Data->ColorMax.g = 255.0f;
	if( Data->ColorMin.g > Data->ColorMax.g )
		Data->ColorMin.g = Data->ColorMax.g;
	Color.g = EffectC_Frand( Data->ColorMin.g, Data->ColorMax.g ) * Data->Intensity;
	if( Data->ColorMin.b < -255.0f )
		Data->ColorMin.b = -255.0f;
	if( Data->ColorMax.b > 255.0f )
		Data->ColorMax.b = 255.0f;
	if( Data->ColorMin.b > Data->ColorMax.b )
		Data->ColorMin.b = Data->ColorMax.b;
	Color.b = EffectC_Frand( Data->ColorMin.b, Data->ColorMax.b ) * Data->Intensity;
	if( Data->ColorMin.a < -255.0f )
		Data->ColorMin.a = -255.0f;
	if( Data->ColorMax.a > 255.0f )
		Data->ColorMax.a = 255.0f;
	if( Data->ColorMin.a > Data->ColorMax.a )
		Data->ColorMin.a = Data->ColorMax.a;
	Color.a = EffectC_Frand( Data->ColorMin.a, Data->ColorMax.a ) * Data->Intensity;
	
	// set radius
	if( Data->RadiusMin < 0.0f )
		Data->RadiusMin = 0.0f;
	if( Data->RadiusMax <= 0.0f )
		Data->RadiusMax = 0.1f;
	if( Data->RadiusMin > Data->RadiusMax )
		Data->RadiusMin = Data->RadiusMax;
	Radius = EffectC_Frand( Data->RadiusMin, Data->RadiusMax );
	
	// adjust the lights parameters
	geWorld_SetLightAttributes(EM->World,
		Data->Light,
		&( Data->Pos ),
		&Color,
		Radius,
		Data->CastShadows );
	
	// all done
	return GE_TRUE;
}

geBoolean EM_Glow_Modify(Eff_Manager *EM, Glow *Data, Glow *NewData, uint32 Flags)
{
	geBoolean RecalculateLeaf = GE_FALSE;
	
	// adjust the source
	if ( Flags & GLOW_POS )
	{
		geVec3d_Copy( &( NewData->Pos ), &( Data->Pos ) );
		RecalculateLeaf = GE_TRUE;
	}
	
	// adjust the min radius
	if ( Flags & GLOW_RADIUSMIN )
	{
		if ( ( NewData->RadiusMin < 0.0f ) || ( NewData->RadiusMin > NewData->RadiusMax ) )
			return GE_FALSE;
		Data->RadiusMin = NewData->RadiusMin;
		if ( Data->RadiusMin > Data->RadiusMax )
			Data->RadiusMax = Data->RadiusMin;
	}
	
	// adjust the max radius
	if ( Flags & GLOW_RADIUSMAX )
	{
		if ( ( NewData->RadiusMax <= 0.0f ) || ( NewData->RadiusMax < NewData->RadiusMin ) )
			return GE_FALSE;
		Data->RadiusMax = NewData->RadiusMax;
		if ( Data->RadiusMax < Data->RadiusMin )
			Data->RadiusMin = Data->RadiusMax;
		
	}
	
	// adjust intensity
	if ( Flags & GLOW_INTENSITY )
	{
		if ( ( NewData->Intensity < 0.0f ) || ( NewData->Intensity > 1.0f ) )
			return GE_FALSE;
		Data->Intensity = NewData->Intensity;
	}
	
	// calculate leaf value
	if ( RecalculateLeaf == GE_TRUE )
		geWorld_GetLeaf(EM->World, &( Data->Pos ), &( Data->Leaf ) );
	
	// all done
	return GE_TRUE;
}

//---------------------------------------------------------------------
// Sprite
//---------------------------------------------------------------------
void *EM_Sprite_Add(Eff_Manager *EM, void *Data)
{
	Sprite	*NewData;
	int i;
	
	NewData = ALLOCATE_STRUCT(Sprite);
	if ( NewData == NULL )
		return NULL;
	
	memcpy( NewData, Data, sizeof( *NewData ) );
	
	NewData->Rotation = 0.0f;
	for ( i = 0; i < 4; i++ )
	{
		NewData->Vertex[i].r = NewData->Color.r;
		NewData->Vertex[i].g = NewData->Color.g;
		NewData->Vertex[i].b = NewData->Color.b;
		NewData->Vertex[i].a = NewData->Color.a;
	}
	NewData->Vertex[0].u = 0.0f;
	NewData->Vertex[0].v = 0.0f;
	NewData->Vertex[1].u = 1.0f;
	NewData->Vertex[1].v = 0.0f;
	NewData->Vertex[2].u = 1.0f;
	NewData->Vertex[2].v = 1.0f;
	NewData->Vertex[3].u = 0.0f;
	NewData->Vertex[3].v = 1.0f;
	NewData->CurrentTexture = 0;
	NewData->ElapsedTime = 0.0f;
	NewData->Direction = 1;
	
	// calculate leaf value
	geWorld_GetLeaf( EM->World, &( NewData->Pos ), &( NewData->Leaf ) );
	
	return (void *)NewData;
}

void EM_Sprite_Remove(Eff_Manager *EM, Sprite *Data)
{
	// free effect data
	free( Data );
	
}

geBoolean EM_Sprite_Process(Eff_Manager *EM, Sprite  *Data,  float  TimeDelta)
{
	int i;
	
	if ( Data->ScaleRate > 0.0f )
	{
		
		// eliminate the effect if the scale has reached zero
		Data->Scale -= ( Data->ScaleRate * TimeDelta );
		if ( Data->Scale <= 0.0f )
		{
			Data->Scale = 0.0f;
			return GE_FALSE;
		}
	}
	// adjust alpha
	if ( Data->AlphaRate > 0.0f )
	{
		
		// eliminate the effect if the alpha has reached zero
		Data->Color.a -= ( Data->AlphaRate * TimeDelta );
		if ( Data->Color.a <= 0.0f )
		{
			Data->Color.a = 0.0f;
			return GE_FALSE;
		}
		
		// adjust all verts with new alpha
		for ( i = 0; i < 4; i++ )
		{
			Data->Vertex[i].a = Data->Color.a;
		}
	}
	// do nothing else if it isn't visible
	if(Data->ShowAlways==GE_FALSE)
	{
		if ( EffectC_IsPointVisible(EM->World,
			EM->Camera,
			&( Data->Pos ),
			Data->Leaf,
			EFFECTC_CLIP_LEAF | EFFECTC_CLIP_SEMICIRCLE ) == GE_FALSE )
			return GE_TRUE;
	}
	// adjust art
	if ( Data->TotalTextures > 1 )
	{
		Data->ElapsedTime += TimeDelta;
		while ( Data->ElapsedTime > Data->TextureRate )
		{
			
			// adjust elapsed time counter
			Data->ElapsedTime -= Data->TextureRate;
			
			// pick the next nexture based on the cycle style
			if ( Data->Style == SPRITE_CYCLE_RANDOM )
			{
				Data->CurrentTexture = ( rand() % Data->TotalTextures );
			}
			else
			{
				
				// pick new texture number
				Data->CurrentTexture += Data->Direction;
				
				// adjust texture number if it has exceeded limits
				if (	( Data->CurrentTexture < 0 ) ||
					( Data->CurrentTexture >= Data->TotalTextures ) )
				{
					if ( Data->Style == SPRITE_CYCLE_REVERSE )
					{
						Data->Direction = -Data->Direction;
						Data->CurrentTexture += Data->Direction;
						Data->CurrentTexture += Data->Direction;
					}
					else if ( Data->Style == SPRITE_CYCLE_RESET )
					{
						Data->CurrentTexture = 0;
					}
					else
					{
						return GE_FALSE;
					}
				}
			}
		}
	}
	// adjust current rotation amount
	if ( Data->RotationRate != 0.0f )
	{
		Data->Rotation += ( Data->RotationRate * TimeDelta );
		if ( Data->Rotation > GE_PI )
		{
			Data->Rotation = -GE_PI;
		}
		else if ( Data->Rotation < -GE_PI )
		{
			Data->Rotation = GE_PI;
		}
	}
	
	// if there is no rotation, then process the sprite this way
	if ( Data->Rotation == 0.0f )
	{
		
		// setup vert
		Data->Vertex[0].X = Data->Pos.X;
		Data->Vertex[0].Y = Data->Pos.Y;
		Data->Vertex[0].Z = Data->Pos.Z;
		
		// update the art
		geWorld_AddPolyOnce(EM->World, Data->Vertex, 1, Data->Texture[Data->CurrentTexture], GE_TEXTURED_POINT, GE_RENDER_DEPTH_SORT_BF, Data->Scale );
	}
	// ...otherwise process it this way
	else
	{
		
		// locals
		const geXForm3d	*CameraXf;
		geXForm3d		NewCameraXf;
		geVec3d			Left, Up, In;
		geQuaternion	Quat;
		float			HalfWidth, HalfHeight;
		
		// determine half width and half height
		HalfWidth = geBitmap_Width( Data->Texture[Data->CurrentTexture] ) * Data->Scale * 0.5f;
		HalfHeight = geBitmap_Height( Data->Texture[Data->CurrentTexture] ) * Data->Scale * 0.5f;
		
		// get left and up vectors from camera transform
		CameraXf = geCamera_GetWorldSpaceXForm(EM->Camera);
		geXForm3d_GetIn( CameraXf, &In );
		geQuaternion_SetFromAxisAngle( &Quat, &In, Data->Rotation );
		geQuaternion_ToMatrix( &Quat, &NewCameraXf );
		geXForm3d_Multiply( &NewCameraXf, CameraXf, &NewCameraXf );
		geXForm3d_GetLeft( &NewCameraXf, &Left );
		geXForm3d_GetUp( &NewCameraXf, &Up );
		geVec3d_Scale( &Left, HalfWidth, &Left );
		geVec3d_Scale( &Up, HalfHeight, &Up );
		
		// setup verticies
		Data->Vertex[0].X = Data->Pos.X + Left.X + Up.X;
		Data->Vertex[0].Y = Data->Pos.Y + Left.Y + Up.Y;
		Data->Vertex[0].Z = Data->Pos.Z + Left.Z + Up.Z;
		Data->Vertex[1].X = Data->Pos.X - Left.X + Up.X;
		Data->Vertex[1].Y = Data->Pos.Y - Left.Y + Up.Y;
		Data->Vertex[1].Z = Data->Pos.Z - Left.Z + Up.Z;
		Data->Vertex[2].X = Data->Pos.X - Left.X - Up.X;
		Data->Vertex[2].Y = Data->Pos.Y - Left.Y - Up.Y;
		Data->Vertex[2].Z = Data->Pos.Z - Left.Z - Up.Z;
		Data->Vertex[3].X = Data->Pos.X + Left.X - Up.X;
		Data->Vertex[3].Y = Data->Pos.Y + Left.Y - Up.Y;
		Data->Vertex[3].Z = Data->Pos.Z + Left.Z - Up.Z;
		
		// update the art
		geWorld_AddPolyOnce(EM->World, Data->Vertex, 4, Data->Texture[Data->CurrentTexture], GE_TEXTURED_POLY, GE_RENDER_DEPTH_SORT_BF, 1.0f );
	}
	
	// all done
	return GE_TRUE;
}

geBoolean EM_Sprite_Modify(Eff_Manager *EM, Sprite *Data, Sprite *NewData, uint32 Flags)
{
	geBoolean RecalculateLeaf = GE_FALSE;
	
	// adjust location
	if ( Flags & SPRITE_POS )
	{
		geVec3d_Copy( &( NewData->Pos ), &( Data->Pos ) );
		RecalculateLeaf = GE_TRUE;
	}
	
	// adjust scale
	if ( Flags & SPRITE_SCALE )
	{
		if ( NewData->Scale <= 0.0f )
			return GE_FALSE;
		Data->Scale = NewData->Scale;
	}
	
	// adjust rotation
	if ( Flags & SPRITE_ROTATION )
	{
		Data->Rotation = NewData->Rotation;
	}
	
	// adjust color
	if ( Flags & SPRITE_COLOR )
	{
		
		// locals
		int	i;
		
		// fail if a bad color was provided
		if ( ( NewData->Color.a < 0.0f ) || ( NewData->Color.a > 255.0f ) )
			return GE_FALSE;
		if ( ( NewData->Color.r < 0.0f ) || ( NewData->Color.r > 255.0f ) )
			return GE_FALSE;
		if ( ( NewData->Color.g < 0.0f ) || ( NewData->Color.g > 255.0f ) )
			return GE_FALSE;
		if ( ( NewData->Color.b < 0.0f ) || ( NewData->Color.b > 255.0f ) )
			return GE_FALSE;
		
		// save new color value
		Data->Color.r = NewData->Color.r;
		Data->Color.g = NewData->Color.g;
		Data->Color.b = NewData->Color.b;
		Data->Color.a = NewData->Color.a;
		
		// apply it to all verts
		for ( i = 0; i < 4; i++ )
		{
			Data->Vertex[i].r = NewData->Color.r;
			Data->Vertex[i].g = NewData->Color.g;
			Data->Vertex[i].b = NewData->Color.b;
			Data->Vertex[i].a = NewData->Color.a;
		}
	}
	
	// recalculate leaf value if required
	if ( RecalculateLeaf == GE_TRUE )
		geWorld_GetLeaf(EM->World, &( Data->Pos ), &( Data->Leaf ) );
	
	// all done
	return GE_TRUE;
}

//---------------------------------------------------------------------
// Sound
//---------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////////////
//
//	Snd_Get3dSoundValues()
//
////////////////////////////////////////////////////////////////////////////////////////
static geBoolean Snd_Get3dSoundValues(Eff_Manager *EM, geEngine *Engine, Snd *Data,
									  geFloat	*Volume,
									  geFloat	*Pan,
									  geFloat	*Frequency )
{
	
	// locals
	const geXForm3d	*SoundXf;
	geFloat	VolDelta, PanDelta;
	
	// get the camera xform
	//@####
	SoundXf = NULL;//&CCD->Player()->ViewPoint();
	
	// get 3d sound values
	geSound3D_GetConfig(
		EM->World,
		SoundXf, 
		&( Data->Pos ), 
		Data->Min, 
		0,
		Volume,
		Pan,
		Frequency );
	
	// return true or false depending on whether or not its worth modifying the sound
	VolDelta = Data->LastVolume - *Volume;
	if ( VolDelta < 0.0f )
	{
		VolDelta = -VolDelta;
	}
	PanDelta = Data->LastPan - *Pan;
	if ( PanDelta < 0.0f )
	{
		PanDelta = -PanDelta;
	}
	if ( ( VolDelta > 0.03f ) || ( PanDelta > 0.02f ) )
	{
		return GE_TRUE;
	}
	return GE_FALSE;
	
} // Snd_Get3dSoundValues()

////////////////////////////////////////////////////////////////////////////////////////
//
//	EM_Snd_Add()
//
////////////////////////////////////////////////////////////////////////////////////////
void *EM_Snd_Add(Eff_Manager *EM, void *Data)
{
	
	// locals
	Snd		*NewData;
	geFloat	Volume;
	geFloat	Pan;
	geFloat	Frequency;
	
	// don't create anything if there is no sound system
	if (EM->AudioSystem == NULL )
		return NULL;
	
	// allocate Snd data
	NewData = ALLOCATE_STRUCT(Snd);
	if ( NewData == NULL )
		return NULL;
	
	memcpy( NewData, Data, sizeof( *NewData ) );
	
	// fail if we have any bad data
	if ( NewData->SoundDef == NULL )
		return NULL;
	if ( NewData->Min < 0.0f )
		return NULL;
	
	// play the sound
	Snd_Get3dSoundValues(EM, EM->Engine, NewData, &Volume, &Pan, &Frequency );
	Volume=0.0f;
	NewData->Sound = geSound_PlaySoundDef(EM->AudioSystem,
		NewData->SoundDef,
		Volume, Pan, Frequency,
		NewData->Loop );
	if ( NewData->Sound == NULL )
	{
		EM_Snd_Remove(EM, NewData );
		return NULL;
	}
	NewData->LastVolume = Volume;
	NewData->LastPan = Pan;
	
	// all done
	return NewData;
} // EM_Snd_Add()

////////////////////////////////////////////////////////////////////////////////////////
//
//	EM_Snd_Remove()
//
////////////////////////////////////////////////////////////////////////////////////////
void EM_Snd_Remove(Eff_Manager *EM, Snd *Data )
{
	// stop the sound
	if ( Data->Sound != NULL )
	{
		geSound_StopSound(EM->AudioSystem, Data->Sound );
	}
	
	// free effect data
	free( Data );
	
} // EM_Snd_Remove()

////////////////////////////////////////////////////////////////////////////////////////
//
//	EM_Snd_Process()
//
//	Perform processing on an indivual effect. A return of GE_FALSE means that the
//	effect needs to be removed.
//
////////////////////////////////////////////////////////////////////////////////////////
geBoolean EM_Snd_Process(Eff_Manager *EM, Snd *Data, float TimeDelta)
{
	
	// locals
	geBoolean	Result;
	geFloat		Volume;
	geFloat		Pan;
	geFloat		Frequency;
	
	// stop the sound if required...
	if ( Data->Sound != NULL )
	{
		// if the sound is done then zap this effect
		if (( Data->Loop == GE_FALSE ) && ( geSound_SoundIsPlaying(EM->AudioSystem, Data->Sound ) == GE_FALSE ) )
			return GE_FALSE;
		
		// adjust the sound if required
		if ( Snd_Get3dSoundValues(EM, EM->Engine, Data, &Volume, &Pan, &Frequency ) == GE_TRUE )
		{
			Result = geSound_ModifySound(EM->AudioSystem,
				Data->Sound,
				Volume, Pan, Frequency );
			if ( Result == GE_FALSE )
				return GE_TRUE;
			Data->LastVolume = Volume;
			Data->LastPan = Pan;
		}
		
		// stop the sound if its volume is out of hearing range
		if ( Data->Loop == GE_TRUE )
		{
			if ( Data->LastVolume < SND_MINAUDIBLEVOLUME )
			{
				geSound_StopSound(EM->AudioSystem, Data->Sound );
				Data->Sound = NULL;
				Data->LastVolume = 0;
				Data->LastPan = 0;
			}
		}
	}
	// ...or restart it
	else
	{
		
		// only restart looping non paused sounds
		if (	( Data->Loop == GE_TRUE ) &&
			( Data->Paused == GE_FALSE ) )
		{
			
			// restart it if its volume is now in hearing range
			Snd_Get3dSoundValues(EM, EM->Engine, Data, &( Data->LastVolume ), &( Data->LastPan ), &Frequency );
			if ( Data->LastVolume >= SND_MINAUDIBLEVOLUME )
			{
				Data->Sound = geSound_PlaySoundDef(EM->AudioSystem,
					Data->SoundDef,
					Data->LastVolume, Data->LastPan, Frequency,
					Data->Loop );
				if( Data->Sound == NULL )
					return GE_TRUE;
				
			}
			else
			{
				Data->LastVolume = 0;
				Data->LastPan = 0;
			}
		}
	}
	
	// all done
	return GE_TRUE;
	
} // EM_Snd_Process()

////////////////////////////////////////////////////////////////////////////////////////
//
//	EM_Snd_Modify()
//
//	Adjust the effect.
//
////////////////////////////////////////////////////////////////////////////////////////
geBoolean EM_Snd_Modify(Eff_Manager *EM, Snd *Data, Snd	*NewData, uint32 Flags )
{
	
	// adjust the position
	if ( Flags & SND_POS )
	{
		
		// save new position
		geVec3d_Copy( &( NewData->Pos ), &( Data->Pos ) );
		
		// adjust the sound
		if ( Data->Sound != NULL )
		{
			
			// locals
			geBoolean	Result;
			geFloat		Volume;
			geFloat		Pan;
			geFloat		Frequency;
			
			// adjust the sound
			if ( Snd_Get3dSoundValues(EM, EM->Engine, Data, &Volume, &Pan, &Frequency ) == GE_TRUE )
			{
				Result = geSound_ModifySound(EM->AudioSystem,
					Data->Sound,
					Volume, Pan, Frequency );
				if( Result == GE_FALSE )
					return GE_FALSE;
				
				Data->LastVolume = Volume;
				Data->LastPan = Pan;
			}
		}
	}
	
	// all done
	return GE_TRUE;
	
} // EM_Snd_Modify()

//---------------------------------------------------------------------
// Bolt
//---------------------------------------------------------------------
void *EM_Bolt_Add(Eff_Manager *EM, void *Data)
{
	Bolt	*NewData;
	Reserved *NewReservedData;
	int i;
	
	NewData = ALLOCATE_STRUCT(Bolt);
	if ( NewData == NULL )
		return NULL;
	
	memcpy( NewData, Data, sizeof( *NewData ) );
	
	NewData->ReservedData = ALLOCATE_STRUCT(Reserved);
	if ( NewData->ReservedData == NULL )
	{
		EM_Bolt_Remove(EM, NewData );
		return NULL;
	}
	NewReservedData = NewData->ReservedData;
	
	if ( NewData->Texture == NULL )
		return NULL;
	if ( NewData->Offset < 0.0f )
		return NULL;
	if ( NewData->SegmentLength <= 0 )
		return NULL;
	if ( NewData->SegmentWidth <= 0.0f )
		return NULL;
	if ( NewData->BoltLimit <= 0 )
		return NULL;
	if ( NewData->CompleteLife < 0.0f )
		return NULL;
	if ( NewData->BoltLife < 0.0f )
		return NULL;
	if ( NewData->BoltCreate < 0.0f )
		return NULL;
	
	// setup reserved data struct
	NewReservedData->BoltDelay = 0.0f;
	NewReservedData->EndReached = GE_FALSE;
	NewReservedData->CurrentBolt = 0;
	NewReservedData->BoltCount = 0;
	NewReservedData->Poly = (struct gePoly **)malloc( NewData->BoltLimit * sizeof( gePoly * ) );
	if ( NewReservedData->Poly == NULL )
	{
		EM_Bolt_Remove(EM, NewData );
		return NULL;
	}
	memset( NewReservedData->Poly, 0, NewData->BoltLimit * sizeof( gePoly * ) );
	
	NewReservedData->BoltLifeList = (float *)malloc( NewData->BoltLimit * sizeof( float ) );
	if ( NewReservedData->BoltLifeList == NULL )
	{
		EM_Bolt_Remove(EM, NewData );
		return NULL;
	}
	memset( NewReservedData->BoltLifeList, 0, NewData->BoltLimit * sizeof( float ) );
	
	// setup default vertex info
	for ( i = 0; i < 4; i++ )
	{
		NewReservedData->Vertex[i].a = NewData->Color.a;
		NewReservedData->Vertex[i].r = NewData->Color.r;
		NewReservedData->Vertex[i].g = NewData->Color.g;
		NewReservedData->Vertex[i].b = NewData->Color.b;
	}
	NewReservedData->Vertex[0].u = 0.0f;
	NewReservedData->Vertex[0].v = 0.0f;
	NewReservedData->Vertex[1].u = 1.0f;
	NewReservedData->Vertex[1].v = 0.0f;
	NewReservedData->Vertex[2].u = 1.0f;
	NewReservedData->Vertex[2].v = 1.0f;
	NewReservedData->Vertex[3].u = 0.0f;
	NewReservedData->Vertex[3].v = 1.0f;
	
	// set current position
	geVec3d_Copy( &( NewData->Start ), &( NewReservedData->Current ) );
	
	// all done
	return (void *)NewData;
}

void EM_Bolt_Remove(Eff_Manager *EM, Bolt *Data)
{
	int32		i;
	Reserved	*ReservedData;
	
	// remove any polys
	ReservedData = Data->ReservedData;
	for ( i = 0; i < Data->BoltLimit; i++ )
	{
		if ( ReservedData->Poly[i] != NULL )
		{
			geWorld_RemovePoly(EM->World, ReservedData->Poly[i] );
		}
	}
	
	// free effect data
	if ( ReservedData->BoltLifeList != NULL )
	{
		free( ReservedData->BoltLifeList );
	}
	if ( ReservedData->Poly != NULL )
	{
		free( ReservedData->Poly );
	}
	if ( Data->ReservedData != NULL )
	{
		free( Data->ReservedData );
	}
	
	// free effect data
	free( Data );
	
}

geBoolean EM_Bolt_Process(Eff_Manager *EM, Bolt  *Data,  float  TimeDelta)
{
	int i;
	Reserved	*ReservedData;
	
	// get reserved data
	ReservedData = Data->ReservedData;
	
	// kill it if required
	if ( Data->CompleteLife > 0.0f )
	{
		Data->CompleteLife -= TimeDelta;
		if ( Data->CompleteLife <= 0.0f )
		{
			return GE_FALSE;
		}
	}
	
	// age all existing bolts
	for ( i = 0; i < Data->BoltLimit; i++ )
	{
		if ( ReservedData->Poly[i] != NULL )
		{
			ReservedData->BoltLifeList[i] -= TimeDelta;
			if ( ReservedData->BoltLifeList[i] <= 0.0f )
			{
				geWorld_RemovePoly(EM->World, ReservedData->Poly[i] );
				ReservedData->Poly[i] = NULL;
				ReservedData->BoltLifeList[i] = 0.0f;
				ReservedData->BoltCount--;
			}
		}
	}
	
	// remove it if all its bolts have expired
	if ( ( ReservedData->BoltCount == 0 ) && ( ReservedData->EndReached == GE_TRUE ) )
	{
		
		// if its the looping type then reset its current position...
		if ( Data->Loop == GE_TRUE )
		{
			geVec3d_Copy( &( Data->Start ), &( ReservedData->Current ) );
			ReservedData->EndReached = GE_FALSE;
			ReservedData->BoltDelay = 0.0f;
		}
		// ...otherwise kill it
		else
		{
			return GE_FALSE;
		}
	}
	
	// update bolt delay time
	ReservedData->BoltDelay += TimeDelta;
	
	// add more bolts if the end has not been reached
	if ( ReservedData->EndReached == GE_FALSE )
	{
		
		//locals
		geVec3d		NextCurrent;
		geXForm3d	Xf;
		float		Distance;
		
		// create as many bolts as required for this time interval
		while ( ReservedData->BoltDelay > 0.0f )
		{
			
			// adjust bolt delay
			ReservedData->BoltDelay -= Data->BoltCreate;
			
			// get current distance to end point	
			Distance = geVec3d_DistanceBetween( &( ReservedData->Current ), &( Data->End ) );
			
			// make end point the next point...
			if ( Distance < Data->SegmentLength )
			{
				
				// mark end of bolt stuff
				ReservedData->BoltDelay = 0.0f;
				ReservedData->EndReached = GE_TRUE;
				
				// setup next point
				geVec3d_Copy( &( Data->End ), &NextCurrent );
				
				// get a transform from current and next bolt points
				EffectC_XFormFromVector( &( ReservedData->Current ), &NextCurrent, &Xf );
			}
			// ...or pick an intermediate point
			else
			{
				
				// locals
				geVec3d		Delta;
				
				// get next bolt point
				geVec3d_Subtract( &( Data->End ), &( ReservedData->Current ), &Delta );
				geVec3d_Normalize( &Delta );
				geVec3d_Scale( &Delta, (float)Data->SegmentLength, &Delta );
				geVec3d_Add( &( ReservedData->Current ), &Delta, &NextCurrent );
				
				// get a transform from current and next bolt points
				EffectC_XFormFromVector( &( ReservedData->Current ), &NextCurrent, &Xf );
				
				// randomly rotate this transform
				geXForm3d_RotateX( &Xf, EffectC_Frand( -3.14159f, 3.14159f ) * Data->Offset );
				geXForm3d_RotateY( &Xf, EffectC_Frand( -3.14159f, 3.14159f ) * Data->Offset );
				geXForm3d_RotateZ( &Xf, EffectC_Frand( -3.14159f, 3.14159f ) * Data->Offset );
				
				// get new next bolt point
				geXForm3d_GetIn( &Xf, &Delta );
				geVec3d_Scale( &Delta, (float)Data->SegmentLength, &Delta );
				geVec3d_Add( &( ReservedData->Current ), &Delta, &NextCurrent );
			}
			
			// THIS IS THE SINGLE BOLT CODE
			{
				
				// locals
				const geXForm3d	*CameraXf;
				geVec3d			In;
				geVec3d			Left;
				gePoly			*PolyA;
				
				// get camera xform
				CameraXf = geCamera_GetWorldSpaceXForm(EM->Camera);
				
				// get left vector
				geXForm3d_GetIn( CameraXf, &In );
				geVec3d_Subtract( &( ReservedData->Current ), &NextCurrent, &Left );
				geVec3d_CrossProduct( &In, &Left, &Left );
				geVec3d_Normalize( &Left );
				geVec3d_Scale( &Left, Data->SegmentWidth / 2.0f, &Left );
				
				// setup verticies
				ReservedData->Vertex[1].X = NextCurrent.X - Left.X;
				ReservedData->Vertex[1].Y = NextCurrent.Y - Left.Y;
				ReservedData->Vertex[1].Z = NextCurrent.Z - Left.Z;
				ReservedData->Vertex[2].X = NextCurrent.X + Left.X;
				ReservedData->Vertex[2].Y = NextCurrent.Y + Left.Y;
				ReservedData->Vertex[2].Z = NextCurrent.Z + Left.Z;
				if ( ReservedData->BoltCount != 0 )
				{
					ReservedData->Vertex[0].X = ReservedData->LastA0.X;
					ReservedData->Vertex[0].Y = ReservedData->LastA0.Y;
					ReservedData->Vertex[0].Z = ReservedData->LastA0.Z;
					ReservedData->Vertex[3].X = ReservedData->LastA3.X;
					ReservedData->Vertex[3].Y = ReservedData->LastA3.Y;
					ReservedData->Vertex[3].Z = ReservedData->LastA3.Z;
				}
				else
				{
					ReservedData->Vertex[0].X = ReservedData->Current.X - Left.X;
					ReservedData->Vertex[0].Y = ReservedData->Current.Y - Left.Y;
					ReservedData->Vertex[0].Z = ReservedData->Current.Z - Left.Z;
					ReservedData->Vertex[3].X = ReservedData->Current.X + Left.X;
					ReservedData->Vertex[3].Y = ReservedData->Current.Y + Left.Y;
					ReservedData->Vertex[3].Z = ReservedData->Current.Z + Left.Z;
				}
				
				// save last end points
				geVec3d_Set( &( ReservedData->LastA0 ), ReservedData->Vertex[1].X, ReservedData->Vertex[1].Y, ReservedData->Vertex[1].Z );
				geVec3d_Set( &( ReservedData->LastA3 ), ReservedData->Vertex[2].X, ReservedData->Vertex[2].Y, ReservedData->Vertex[2].Z );
				
				// setup color
				for ( i = 0; i < 4; i++ )
				{
					ReservedData->Vertex[i].a = Data->Color.a;
					ReservedData->Vertex[i].r = Data->Color.r;
					ReservedData->Vertex[i].g = Data->Color.g;
					ReservedData->Vertex[i].b = Data->Color.b;
				}
				
				// add the poly to the world
				PolyA = NULL;
				PolyA = geWorld_AddPoly(EM->World, ReservedData->Vertex, 4, Data->Texture, GE_TEXTURED_POLY, GE_RENDER_DEPTH_SORT_BF, 1.0f );
				
				// remove other polys to accomodate the new one if required
				if ( ReservedData->Poly[ReservedData->CurrentBolt] != NULL )
				{
					geWorld_RemovePoly(EM->World, ReservedData->Poly[ReservedData->CurrentBolt] );
					ReservedData->BoltCount--;
				}
				ReservedData->Poly[ReservedData->CurrentBolt] = PolyA;
				ReservedData->BoltLifeList[ReservedData->CurrentBolt] = Data->BoltLife;
				ReservedData->BoltCount++;
				ReservedData->CurrentBolt++;
				if ( ReservedData->CurrentBolt >= Data->BoltLimit )
				{
					ReservedData->CurrentBolt = 0;
				}
				
				// the last location
				geVec3d_Copy( &NextCurrent, &( ReservedData->Current ) );
			}
		}
	}
	
	// all done
	return GE_TRUE;
}

geBoolean EM_Bolt_Modify(Eff_Manager *EM, Bolt *Data, Bolt *NewData, uint32 Flags)
{
	// adjust start
	if ( Flags & BOLT_START )
	{
		geVec3d_Copy( &( NewData->Start ), &( Data->Start ) );
	}
	
	// adjust end
	if ( Flags & BOLT_END )
	{
		geVec3d_Copy( &( NewData->End ), &( Data->End ) );
	}
	
	// adjust color
	if ( Flags & BOLT_COLOR )
	{
		memcpy( &( Data->Color ), &( NewData->Color ), sizeof( Data->Color ) );
	}
	
	// all done
	return GE_TRUE;
}

void EM_Bolt_Pause(Eff_Manager *EM, Bolt *Data, geBoolean Pause )
{
	// locals
	Reserved	*ReservedData;
	int32		i;
	
	// get reserved data
	ReservedData = Data->ReservedData;
	
	// remove all polys if required
	if ( Pause == GE_TRUE )
	{
		for ( i = 0; i < Data->BoltLimit; i++ )
		{
			if ( ReservedData->Poly[i] != NULL )
			{
				geWorld_RemovePoly(EM->World, ReservedData->Poly[i] );
				ReservedData->Poly[i] = NULL;
				ReservedData->BoltCount--;
			}
		}
	}
	
}

//---------------------------------------------------------------------
// Corona
//---------------------------------------------------------------------
void *EM_Corona_Add(Eff_Manager *EM, void *Data)
{
	EffCorona	*NewData;
	geVec3d	Pos;
	
	NewData = ALLOCATE_STRUCT(EffCorona);
	if ( NewData == NULL )
		return NULL;
	
	memcpy( NewData, Data, sizeof( *NewData ) );
	
	if ( NewData->Texture == NULL )
		return NULL;
	if( NewData->FadeTime < 0.0f )
		return NULL;
	if ( NewData->MaxVisibleDistance <= 0.0f )
		return NULL;
	if( NewData->MinRadius < 0.0f )
		return NULL;
	if ( NewData->MaxRadius < NewData->MinRadius )
		return NULL;
	if ( NewData->MinRadiusDistance < 0.0f )
		return NULL;
	if ( ( NewData->MaxRadiusDistance <= 0.0f ) || ( NewData->MaxRadiusDistance < NewData->MinRadiusDistance ) )
		return NULL;
	
	// setup defaultdata
	NewData->Vertex.u = NewData->Vertex.v = 0.0f;
	NewData->LastVisibleRadius = 0.0f;
	
	// calculate leaf value
	geVec3d_Set( &Pos, NewData->Vertex.X, NewData->Vertex.Y, NewData->Vertex.Z );
	geWorld_GetLeaf(EM->World, &Pos, &( NewData->Leaf ) );
	
	// all done
	return (void *)NewData;
}

void EM_Corona_Remove(Eff_Manager *EM, EffCorona *Data)
{
	// free effect data
	free( Data );
	
}

geBoolean EM_Corona_Process(Eff_Manager *EM, EffCorona  *Data,  float  TimeDelta)
{
	// locals
	const geXForm3d	*CameraXf;
	geVec3d			Pos;
	geVec3d			Delta;
	geFloat			DistanceToCorona;
	geBoolean		Visible;
	
	// get camera xform
	CameraXf = geCamera_GetWorldSpaceXForm(EM->Camera);
	
	// determine distance to corona
	geVec3d_Set( &Pos, Data->Vertex.X, Data->Vertex.Y, Data->Vertex.Z );
	geVec3d_Subtract( &Pos, &( CameraXf->Translation ), &Delta);
	DistanceToCorona = geVec3d_Length( &Delta );
	
	// determine distance to corona
	Visible = EffectC_IsPointVisible(EM->World,
		EM->Camera,
		&Pos,
		Data->Leaf,
		EFFECTC_CLIP_LEAF | EFFECTC_CLIP_LINEOFSIGHT | EFFECTC_CLIP_SEMICIRCLE );
	// process the corona if required
	if ( Data->Paused == GE_FALSE )
	{
		// set new radius
		if ( Visible )
		{
			// locals
			float	DesiredRadius;
			
			// determine desired radius
			if ( DistanceToCorona >= Data->MaxRadiusDistance )
			{
				DesiredRadius = Data->MaxRadius;
			}
			else if	( DistanceToCorona <= Data->MinRadiusDistance )
			{
				DesiredRadius = Data->MinRadius;
			}
			else
			{
				
				// locals
				geFloat	Slope;
				
				// determine radius
				Slope = ( Data->MaxRadius - Data->MinRadius ) / ( Data->MaxRadiusDistance - Data->MinRadiusDistance );
				DesiredRadius = Data->MinRadius + Slope * ( DistanceToCorona - Data->MinRadiusDistance );
			}
			
			// scale radius upwards
			if ( Data->FadeTime > 0.0f )
			{
				Data->LastVisibleRadius += ( ( TimeDelta * Data->MaxRadius ) / Data->FadeTime );
				if ( Data->LastVisibleRadius > DesiredRadius )
				{
					Data->LastVisibleRadius = DesiredRadius;
				}
			}
			else
			{
				Data->LastVisibleRadius = DesiredRadius;
			}
		}
		else if ( Data->LastVisibleRadius > 0.0f )
		{
			
			// scale radius down
			if ( Data->FadeTime > 0.0f )
			{
				Data->LastVisibleRadius -= ( ( TimeDelta * Data->MaxRadius ) / Data->FadeTime );
				if ( Data->LastVisibleRadius < 0.0f )
				{
					Data->LastVisibleRadius = 0.0f;
				}
			}
			else
			{
				Data->LastVisibleRadius = 0.0f;
			}
		}
		
		// update the art
		if ( Data->LastVisibleRadius > 0.0f )
		{
			geWorld_AddPolyOnce(EM->World,
				&( Data->Vertex ),
				1,
				Data->Texture,
				GE_TEXTURED_POINT,
				GE_RENDER_DO_NOT_OCCLUDE_SELF,
				Data->LastVisibleRadius );
		}
	}
	
	// all done
	return GE_TRUE;
}

geBoolean EM_Corona_Modify(Eff_Manager *EM, EffCorona *Data, EffCorona *NewData, uint32 Flags)
{
	// locals
	geBoolean	RecalculateLeaf = GE_FALSE;
	
	// adjust location
	if ( Flags & CORONA_POS )
	{
		Data->Vertex.X = NewData->Vertex.X;
		Data->Vertex.Y = NewData->Vertex.Y;
		Data->Vertex.Z = NewData->Vertex.Z;
		RecalculateLeaf = GE_TRUE;
	}
	
	// recalculate leaf value if required
	if ( RecalculateLeaf == GE_TRUE )
	{
		
		// locals
		geVec3d		Pos;
		
		// recalc leaf
		geVec3d_Set( &Pos, NewData->Vertex.X, NewData->Vertex.Y, NewData->Vertex.Z );
		geWorld_GetLeaf(EM->World, &Pos, &( Data->Leaf ) );
	}
	
	// all done
	return GE_TRUE;
}

//---------------------------------------------------------------------
// Blood
//---------------------------------------------------------------------
void *EM_Blood_Add(Eff_Manager *EM, void *Data)
{
	Blood	*NewData;
	
	NewData = ALLOCATE_STRUCT(Blood);
	if ( NewData == NULL )
		return NULL;
	memcpy( NewData, Data, sizeof( *NewData ) );
	
	return (void *)NewData;
}

void EM_Blood_Remove(Eff_Manager *EM, Blood *Data)
{
	// free effect data
	free( Data );	
}

#define		COLLISION		geWorld_Collision(EM->World,&Data->ExtBox.Min,&Data->ExtBox.Max,&Data->Position,&newPos,GE_CONTENTS_SOLID_CLIP | GE_VISIBLE_CONTENTS,GE_COLLIDE_MODELS ,0xffffffff,0,NULL,&lCol)
geBoolean EM_Blood_Process(Eff_Manager *EM, Blood  *Data,  float TimeDelta)
{
	geVec3d newPos;
	GE_Collision lCol;

	geVec3d_AddScaled(&Data->Velocity, &Data->Gravity, TimeDelta, &Data->Velocity);
	geVec3d_AddScaled(&Data->Position, &Data->Velocity, TimeDelta, &newPos);
	
	if( COLLISION ){
		geVec3d In;

		geXForm3d_GetIn(geCamera_GetWorldSpaceXForm(EM->Camera), &In);
		DecalMgr_AddDecal(Data->mgr, DecalMgr_GetRandomDecal(Data->decalType), -1.0f, Data->decalColor, 1.0f,
					   &lCol.Impact, &lCol.Plane.Normal, &In);

		return GE_FALSE;
	} else {
		
		GE_LVertex vertex;
		// update position
		Data->Position = newPos;
		// setup default vertex data
		vertex.u = 0.0f;
		vertex.v = 0.0f;
		vertex.r = Data->Color.r;
		vertex.g = Data->Color.g;
		vertex.b = Data->Color.b;
		vertex.a = Data->Color.a;

		vertex.X = Data->Position.X;
		vertex.Y = Data->Position.Y;
		vertex.Z = Data->Position.Z;

		geWorld_AddPolyOnce(EM->World,
				&( vertex ),
				1,
				Data->Texture,
				GE_TEXTURED_POINT,
				0,
				Data->TextureScale );
	}

	Data->life -= TimeDelta;
	
	// all done
	if( Data->life <= 0.0f ) return GE_FALSE;
	return GE_TRUE;
}
#undef COLLISION

geBoolean EM_Blood_Modify(Eff_Manager *EM, Blood *Data, Blood *NewData, uint32 Flags)
{
	// all done
	return GE_TRUE;
}


////////////////////////////////////////////////////////////////////////////////////////
//
//	EffectC_Frand()
//
//	Picks a random float within the supplied range.
//
////////////////////////////////////////////////////////////////////////////////////////
static float EffectC_Frand(float Low, float High )
{
	
	// locals
	float	Range;
	
	// if they are the same then just return one of them
	if ( High == Low )
	{
		return Low;
	}
	
	// pick a random float from whithin the range
	Range = High - Low;
	return ( (float)( ( ( rand() % 1000 ) + 1 ) ) ) / 1000.0f * Range + Low;
	
} // EffectC_Frand()



////////////////////////////////////////////////////////////////////////////////////////
//
//	EffectC_XFormFromVector()
//
//	Create a transform from two vectors.
//
////////////////////////////////////////////////////////////////////////////////////////
static void EffectC_XFormFromVector(geVec3d *Source, geVec3d *Target, geXForm3d *Out )
{
	
	// locals
	geVec3d	Temp, Vertical, Vect;
	
	// create a straight up vector
	Vertical.X = 0.0f;
	Vertical.Y = 1.0f;
	Vertical.Z = 0.0f;
	
	// create the source vector, fudging it if its coplanar to the comparison vector
	geVec3d_Subtract( Source, Target, &Vect );
	if ( ( Vertical.X == Vect.X ) && ( Vertical.Z == Vect.Z ) )
	{
		Vertical.X += 1.0f;
	}
	
	// set the IN vector
	geXForm3d_SetIdentity( Out );
	geVec3d_Normalize( &Vect );
	Out->AZ = Vect.X;
	Out->BZ = Vect.Y;
	Out->CZ = Vect.Z;
	
	// use it with the in vector to get the RIGHT vector
	geVec3d_CrossProduct( &Vertical, &Vect, &Temp );
	geVec3d_Normalize( &Temp );
	
	// put the RIGHT vector in the matrix
	Out->AX = Temp.X;
	Out->BX = Temp.Y;
	Out->CX = Temp.Z;
	
	// use the RIGHT vector with the IN vector to get the real UP vector
	geVec3d_CrossProduct( &Vect, &Temp, &Vertical );
	geVec3d_Normalize( &Vertical );
	
	// put the UP vector in the matrix
	Out->AY = Vertical.X;
	Out->BY = Vertical.Y;
	Out->CY = Vertical.Z;
	
	// put the translation in
	Out->Translation = *Source;
	
} // EffectC_XFormFromVector()



////////////////////////////////////////////////////////////////////////////////////////
//
//	EffectC_IsPointVisible()
//
//	Returns true if point is visible, false if it isn't.
//
////////////////////////////////////////////////////////////////////////////////////////
static geBoolean EffectC_IsPointVisible(geWorld *World, geCamera *Camera, geVec3d *Target, int32 Leaf,	uint32 ClipStyle )
{
	
	// leaf check
	if ( ClipStyle & EFFECTC_CLIP_LEAF )
	{
		if ( geWorld_MightSeeLeaf( World, Leaf ) == GE_FALSE )
		{
			return GE_FALSE;
		}
	}
	
	// semi circle check
	if ( ClipStyle & EFFECTC_CLIP_SEMICIRCLE )
	{
		
		// locals
		const geXForm3d	*CameraXf;
		geVec3d			In;
		geVec3d			Delta;
		float			Dot;
		
		// get camera xform
		CameraXf = geCamera_GetWorldSpaceXForm( Camera );
		
		// get angle between camera in vector and vector to target
		geVec3d_Subtract( Target, &( CameraXf->Translation ), &Delta );
		geVec3d_Normalize( &Delta );
		geXForm3d_GetIn( CameraXf, &In );
		Dot = geVec3d_DotProduct( &In, &Delta );
		
		// check if its visible
		if ( Dot < 0.0f )
		{
			return GE_FALSE;
		}
	}
	
	// line of sight check
	if ( ClipStyle & EFFECTC_CLIP_LINEOFSIGHT )
	{
		
		// locals
		GE_Collision	Collision;
		const geXForm3d	*CameraXf;
		
		// get camera xform
		CameraXf = geCamera_GetWorldSpaceXForm( Camera );
		
		// check if its visible
		if ( geWorld_Collision( World, (const geVec3d *)NULL, (const geVec3d *)NULL, &( CameraXf->Translation ), Target, GE_CONTENTS_SOLID, GE_COLLIDE_MODELS, 0, (GE_CollisionCB *)NULL, NULL, &Collision ) == GE_TRUE )
		{
			return GE_FALSE;
		}
	}
	
	// if we got to here then its visible
	return GE_TRUE;
	
} // EffectC_IsPointVisible()





























//---------------------------------------------------------------------
// Bolt
//---------------------------------------------------------------------

Electric_BoltEffect * Electric_BoltEffectCreate(
	geBitmap				*Texture,	/* The texture we map onto the bolt */
	geBitmap				*Texture2,	/* The texture we map onto the bolt */
	int 					NumPolys,	/* Number of polys, must be power of 2 */
	int						Width,		/* Width in world units of the bolt */
	geFloat 				Wildness);	/* How wild the bolt is (0 to 1 inclusive) */

void Electric_BoltEffectDestroy(Electric_BoltEffect *Effect);

void Electric_BoltEffectAnimate(
	Electric_BoltEffect *	Effect,
	const geVec3d *			start,		/* Starting point of the bolt */
	const geVec3d *			end);		/* Ending point of the bolt */

void Electric_BoltEffectRender(
	geWorld *				World,		/* World to render for */
	Electric_BoltEffect *	Effect,		/* Bolt to render */
	const geXForm3d *		XForm);		/* Transform of our point of view */

void Electric_BoltEffectSetColorInfo(
	Electric_BoltEffect *	Effect,
	GE_RGBA *				BaseColor,		/* Base color of the bolt (2 colors should be the same */
	int						DominantColor);	/* Which color is the one to leave fixed */

static int logBase2(int n) {
	int	i = 0;

	assert(n != 0);

	while	(!(n & 1)) {
		n = n >> 1;
		i++;
	}
	assert((n & ~1) == 0);
	return i;
}

static geBoolean IsPowerOf2(int n) {
	if	(n == 0)
		return GE_TRUE;
	while	(!(n & 1))
		n = n >> 1;
	if	(n & ~1)
		return GE_FALSE;
	return GE_TRUE;
}

Electric_BoltEffect * Electric_BoltEffectCreate(
	geBitmap		*Bitmap,
	geBitmap		*Bitmap2,
 	int NumPolys,
 	int Width,
	geFloat Wildness)
{
	Electric_BoltEffect *	be;
	GE_RGBA					color;

	assert(Wildness >= 0.0f && Wildness <= 1.0f);

	/* Asserts power of 2 */
	logBase2(NumPolys);

	be = (Electric_BoltEffect *)malloc(sizeof(*be));
	if	(!be)
		return be;

	memset(be, 0, sizeof(*be));

	be->beCenterPoints = (geVec3d *)malloc(sizeof(*be->beCenterPoints) * (NumPolys + 1));
	if	(!be->beCenterPoints)
		goto fail;

	be->beBitmap	= Bitmap;
//	be->beBitmap2	= Bitmap2;
	be->beNumPoints	= NumPolys;
	be->beWildness	= Wildness;
	be->beWidth		= Width;

//	color.r = 255.0f;
//	color.g = 60.0f;
//	color.b = 60.0f;
//	Electric_BoltEffectSetColorInfo(be, &color, ELECTRIC_BOLT_REDDOMINANT);

//	color.r = 60.0f;
//	color.g = 255.0f;
//	color.b = 60.0f;
//	Electric_BoltEffectSetColorInfo(be, &color, ELECTRIC_BOLT_GREENDOMINANT);

	color.r = 160.0f;
	color.g = 160.0f;
	color.b = 255.0f;
	Electric_BoltEffectSetColorInfo(be, &color, ELECTRIC_BOLT_BLUEDOMINANT);

	return be;

fail:
	if	(be->beCenterPoints)
		free(be->beCenterPoints);
	return NULL;
}

void Electric_BoltEffectDestroy(Electric_BoltEffect *Effect) {
	free(Effect->beCenterPoints);
	free(Effect);
}

static geFloat GaussRand(void) {
	int	i;
	int	r;

	r = 0;

	for	(i = 0; i < 6; i++)
		r = r + rand() - rand();

	return (geFloat)r / ((geFloat)RAND_MAX * 6.0f);
}

static void subdivide(
	Electric_BoltEffect *	be,
	const geVec3d *			start,
	const geVec3d *			end,
	geFloat 				s,
	int 					n)
{
	geVec3d	tmp;

	if	(n == 0) {
		be->beCurrentPoint++;
		*be->beCurrentPoint = *end;
		return;
	}
	
	tmp.X = (end->X + start->X) / 2 + s * GaussRand();
	tmp.Y = (end->Y + start->Y) / 2 + s * GaussRand();
	tmp.Z = (end->Z + start->Z) / 2 + s * GaussRand();
	subdivide(be,  start, &tmp, s / 2, n - 1);
	subdivide(be, &tmp,    end, s / 2, n - 1);
}

#define	LIGHTNINGWIDTH 8.0f

static	void	genLightning(
	Electric_BoltEffect *	be,
	int 					RangeLow,
	int 					RangeHigh,
	const geVec3d *			start,
	const geVec3d *			end)
{
	geFloat	length;
	int		seed;

	assert(be);
	assert(start);
	assert(end);
	assert(RangeHigh > RangeLow);
	assert(IsPowerOf2(RangeHigh - RangeLow));

	/* Manhattan length is good enough for this */
	length = (geFloat)(fabs(start->X - end->X) +
						fabs(start->Y - end->Y) +
						fabs(start->Z - end->Z));

	seed = rand();

	srand(seed);
	be->beCurrentPoint					= be->beCenterPoints + RangeLow;
	be->beCenterPoints[RangeLow]		= *start;
	be->beCenterPoints[RangeHigh] 		= *end;
//	be->beCenterPoints[be->beNumPoints] = *end;
//	subdivide(be, start, end, length * be->beWildness, logBase2(be->beNumPoints));
	subdivide(be, start, end, length * be->beWildness, logBase2(RangeHigh - RangeLow));
}

void Electric_BoltEffectSetColorInfo(
	Electric_BoltEffect *	Effect,
	GE_RGBA *				BaseColor,
	int						DominantColor)
{
	Effect->beBaseColors[0]		= BaseColor->r;
	Effect->beBaseColors[1]		= BaseColor->g;
	Effect->beBaseColors[2]		= BaseColor->b;
	Effect->beCurrentColors[0]	= BaseColor->r;
	Effect->beCurrentColors[1]	= BaseColor->g;
	Effect->beCurrentColors[2]	= BaseColor->b;
	Effect->beDominantColor 	= DominantColor;
}

void Electric_BoltEffectAnimate(
	Electric_BoltEffect *	Effect,
	const geVec3d *			start,
	const geVec3d *			end)
{
	int		dominant;
	int		nonDominant1;
	int		nonDominant2;
	geVec3d	SubdivideStart;
	geVec3d	SubdivideEnd;
	int		LowIndex;
	int		HighIndex;

	Effect->beStart = *start;
	Effect->beEnd	= *end;

	dominant = Effect->beDominantColor;
	nonDominant1 = (dominant + 1) % 3;
	nonDominant2 = (dominant + 2) % 3;
	if	(Effect->beBaseColors[nonDominant1] == Effect->beCurrentColors[nonDominant1]) {
		int	DecayRate;
		int	Spike;

		DecayRate = rand() % (int)(Effect->beBaseColors[dominant] - Effect->beBaseColors[nonDominant1]);
		DecayRate = max(DecayRate, 5);
		Effect->beDecayRate = DecayRate;
		if	(Effect->beBaseColors[nonDominant1] >= 1.0f)
			Spike = rand() % (int)(Effect->beBaseColors[nonDominant1]);
		else
			Spike = 0;
		Effect->beCurrentColors[nonDominant1] -= Spike;
		Effect->beCurrentColors[nonDominant2] -= Spike;
	}
	else {
		Effect->beCurrentColors[nonDominant1] += Effect->beDecayRate;
		Effect->beCurrentColors[nonDominant2] += Effect->beDecayRate;
		if	(Effect->beCurrentColors[nonDominant1] > Effect->beBaseColors[nonDominant1])
		{
			Effect->beCurrentColors[nonDominant1] = Effect->beBaseColors[nonDominant1];
			Effect->beCurrentColors[nonDominant2] = Effect->beBaseColors[nonDominant2];
		}
	}

	if	(Effect->beInitialized && Effect->beNumPoints > 16) {
		int		P1;
		int		P2;
		int		P3;
		int		P4;

		switch	(rand() % 7) {
			case	0:
				genLightning(Effect, 0, Effect->beNumPoints, start, end);
				return;

			case	1:
			case	2:
			case	3:
				P1 = 0;
				P2 = Effect->beNumPoints / 2;
				P3 = P2 + Effect->beNumPoints / 4;
				P4 = Effect->beNumPoints;
				break;

			case	4:
			case	5:
			case	6:
				P1 = 0;
				P3 = Effect->beNumPoints / 2;
				P2 = P3 - Effect->beNumPoints / 4;
				P4 = Effect->beNumPoints;
				break;
		}
		SubdivideStart = Effect->beCenterPoints[P1];
		SubdivideEnd = Effect->beCenterPoints[P2];
		genLightning(Effect, P1, P2, &SubdivideStart, &SubdivideEnd);
		SubdivideStart = Effect->beCenterPoints[P2];
		SubdivideEnd = Effect->beCenterPoints[P3];
		genLightning(Effect, P2, P3, &SubdivideStart, &SubdivideEnd);
		SubdivideStart = Effect->beCenterPoints[P3];
		SubdivideEnd = Effect->beCenterPoints[P4];
		genLightning(Effect, P3, P4, &SubdivideStart, &SubdivideEnd);
	}
	else
	{
		Effect->beInitialized = 1;
		LowIndex = 0;
		HighIndex = Effect->beNumPoints;
		SubdivideStart = *start;
		SubdivideEnd   = *end;

		genLightning(Effect, LowIndex, HighIndex, &SubdivideStart, &SubdivideEnd);
	}
}

#if 0
static	void	DrawPoint(geWorld *world, geVec3d *pos, geBitmap *Bitmap, int r, int g, int b)
{
	GE_LVertex	vert;

	vert.X = pos->X;
	vert.Y = pos->Y;
	vert.Z = pos->Z;
	vert.r = (geFloat)r;
	vert.g = (geFloat)g;
	vert.b = (geFloat)b;
	vert.a = 255.0f;
	vert.u = vert.v = 0.0f;

	GE_WorldAddPolyOnce(world,
						&vert,
						1,
						Bitmap,
						GE_TEXTURED_POINT,
						GE_FX_TRANSPARENT,
						EffectScale);
}
#endif

#define	LIGHTNINGALPHA	160.0f
#define	LIGHTNINGSTROKEDURATION	0.05f

void Electric_BoltEffectRender(
	geWorld *				World,
	Electric_BoltEffect *	be,
	const geXForm3d *		XForm)
{
	geVec3d			perp;
	geVec3d			temp;
	geVec3d			in;
	GE_LVertex 		verts[4];
	int				i;

	geVec3d_Subtract(&be->beStart, &be->beEnd, &temp);
	geXForm3d_GetIn(XForm, &in);

	geVec3d_CrossProduct(&in, &temp, &perp);
	geVec3d_Normalize(&perp);

	geVec3d_Scale(&perp, be->beWidth / 2.0f, &perp);

	/*
		We've got the perpendicular to the camera in the
		rough direction of the electric bolt center.  Walk
		the left and right sides, constructing verts, then
		do the drawing.
	*/
	for	(i = 0; i < be->beNumPoints - 1; i++) {
		geVec3d	temp;

		geVec3d_Subtract(&be->beCenterPoints[i], &perp, &temp);
		verts[0].X = temp.X;
		verts[0].Y = temp.Y;
		verts[0].Z = temp.Z;
		verts[0].u = 0.0f;
		verts[0].v = 0.0f;
		verts[0].r = be->beCurrentColors[0];
		verts[0].g = be->beCurrentColors[1];
		verts[0].b = be->beCurrentColors[2];
		verts[0].a = LIGHTNINGALPHA;

		geVec3d_Subtract(&be->beCenterPoints[i + 1], &perp, &temp);
		verts[1].X = temp.X;
		verts[1].Y = temp.Y;
		verts[1].Z = temp.Z;
		verts[1].u = 0.0f;
		verts[1].v = 1.0f;
		verts[1].r = be->beCurrentColors[0];
		verts[1].g = be->beCurrentColors[1];
		verts[1].b = be->beCurrentColors[2];
		verts[1].a = LIGHTNINGALPHA;

		geVec3d_Add(&be->beCenterPoints[i + 1], &perp, &temp);
		verts[2].X = temp.X;
		verts[2].Y = temp.Y;
		verts[2].Z = temp.Z;
		verts[2].u = 1.0f;
		verts[2].v = 1.0f;
		verts[2].r = be->beCurrentColors[0];
		verts[2].g = be->beCurrentColors[1];
		verts[2].b = be->beCurrentColors[2];
		verts[2].a = LIGHTNINGALPHA;

		geVec3d_Add(&be->beCenterPoints[i], &perp, &temp);
		verts[3].X = temp.X;
		verts[3].Y = temp.Y;
		verts[3].Z = temp.Z;
		verts[3].u = 1.0f;
		verts[3].v = 0.0f;
		verts[3].r = be->beCurrentColors[0];
		verts[3].g = be->beCurrentColors[1];
		verts[3].b = be->beCurrentColors[2];
		verts[3].a = LIGHTNINGALPHA;

		geWorld_AddPolyOnce(World,
							verts,
							4,
							be->beBitmap,
							GE_TEXTURED_POLY,
							GE_RENDER_DO_NOT_OCCLUDE_OTHERS,
							1.0f);

//		DrawPoint(World, &be->beCenterPoints[i], be->beTexture, 255, 0, 0);
	}
}

static geFloat frand(geFloat Low, geFloat High) {
	geFloat	Range;
	Range = High - Low;
	return ((geFloat)(((rand() % 1000) + 1))) / 1000.0f * Range + Low;
}

void *EM_EBolt_Add(Eff_Manager *EM, void *Data) {
	ElectricBolt* bolt;
	bolt = (ElectricBolt*) malloc( sizeof(ElectricBolt) );
	if( !bolt) return 0;
	memcpy(bolt, Data, sizeof(ElectricBolt) );
	bolt->Bolt = Electric_BoltEffectCreate(bolt->Texture, NULL, bolt->NumPoints, bolt->Width, bolt->Wildness);
	if( bolt->Bolt == NULL ) {
		free(bolt);
		return NULL;
	}
	Electric_BoltEffectSetColorInfo(bolt->Bolt, &bolt->Color, bolt->DominantColor);
	return bolt;
}

void EM_EBolt_Remove(Eff_Manager *EM, ElectricBolt *bolt) {	
	Electric_BoltEffectDestroy(bolt->Bolt);
	bolt->Bolt = 0;
	free(bolt);
}

geBoolean EM_EBolt_Process(Eff_Manager *EM, ElectricBolt  *bolt,  float  TimeDelta) {
	int32 Leaf;
	geVec3d MidPoint;
	const geXForm3d		*CameraXf;
	CameraXf = geCamera_GetWorldSpaceXForm( EM->Camera );

	geVec3d_Subtract(&bolt->Terminus, &bolt->origin, &MidPoint);
	geVec3d_AddScaled(&bolt->origin, &MidPoint, 0.5f, &MidPoint);
	geWorld_GetLeaf(EM->World, &MidPoint, &Leaf);

	if (geWorld_MightSeeLeaf(EM->World, Leaf))	{
		bolt->LastTime += TimeDelta;

		if	(!bolt->Intermittent ||  (bolt->LastTime - bolt->LastBoltTime > frand(bolt->MaxFrequency, bolt->MinFrequency))) {
			Electric_BoltEffectAnimate(bolt->Bolt,
									   &bolt->origin,
									   &bolt->Terminus);
			bolt->LastBoltTime = bolt->LastTime;
		}

		if	(bolt->LastTime - bolt->LastBoltTime <= LIGHTNINGSTROKEDURATION)
			Electric_BoltEffectRender(EM->World, bolt->Bolt, CameraXf);

		if ( ! bolt->imortal ) {
			bolt->life -= TimeDelta;
			if( bolt->life < 0 ) return GE_FALSE;
		}
	}

	return GE_TRUE;
}
void EM_EBolt_Pause(Eff_Manager *EM, ElectricBolt *Data, geBoolean Pause ) {	
}