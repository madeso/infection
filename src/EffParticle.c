/******************************************************************************/
/*  EffParticle.c                                                             */
/*                                                                            */
/*  Author: David Wulff, Eli Boling, Peter Siamidis                           */
/*  Description: Partical System Manager                                      */
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
#include <memory.h>
#include <malloc.h>
#include "genesis.h"
#include "Ram.h"

#include "EffParticle.h"


#include "useful_macros.h"

//#define	POLYQ

#define	PARTICLE_USER			( 1 << 0 )
#define	PARTICLE_HASVELOCITY	( 1 << 1 )
#define PARTICLE_HASGRAVITY		( 1 << 2 )
#define PARTICLE_RENDERSTYLE	GE_RENDER_DEPTH_SORT_BF | GE_RENDER_DO_NOT_OCCLUDE_OTHERS


void Particle_SetTexture(Particle *p, geBitmap *Texture)
{
	p->ptclTexture = Texture;
}

GE_LVertex *  Particle_GetVertex(Particle *p)
{
	return &p->ptclVertex;
}

#define	QUANTUMSIZE	(1.0f / 30.0f)

Particle_System *  Particle_SystemCreate(geWorld *World)
{
	Particle_System *ps;
	
	ps = ALLOCATE_STRUCT(Particle_System);
	if	(!ps)
		return ps;
	
	memset(ps, 0, sizeof(*ps));
	
	ps->psWorld	= World;
	ps->psQuantumSeconds = QUANTUMSIZE;
	ps->psLastTime = 0.0f;
	
	return ps;
}

static	void 	DestroyParticle(Particle_System *ps, Particle *p)
{
	if	(p->ptclPoly)
		geWorld_RemovePoly(ps->psWorld, p->ptclPoly);
	free(p);
}

void 	Particle_SystemDestroy(Particle_System *ps)
{
	Particle *	ptcl;
	
	ptcl = ps->psParticles;

	while	(ptcl)
	{
		Particle *	temp;
		
		temp = ptcl->ptclNext;
		DestroyParticle(ps, ptcl);
		ptcl = temp;
	}
	//geRam_Free_(ps);
	free(ps);
}

static	void 	UnlinkParticle(Particle_System *ps, Particle *ptcl)
{
	if	(ptcl->ptclPrev)
		ptcl->ptclPrev->ptclNext = ptcl->ptclNext;
	
	if	(ptcl->ptclNext)
		ptcl->ptclNext->ptclPrev = ptcl->ptclPrev;
	
	if	(ps->psParticles == ptcl)
		ps->psParticles = ptcl->ptclNext;
}

void  Particle_SystemRemoveAll(Particle_System *ps)
{
	Particle *	ptcl;
	
	ptcl = ps->psParticles;
	while	(ptcl)
	{
		Particle *	temp;
		
		temp = ptcl->ptclNext;
		UnlinkParticle(ps, ptcl);
		DestroyParticle(ps, ptcl);
		ptcl = temp;
	}
}

int32 	Particle_GetCount(Particle_System *ps)
{
	
	// locals
	Particle	*ptcl;
	int32		TotalParticleCount = 0;
	
	// count up how many particles are active in this particle system
	ptcl = ps->psParticles;
	while ( ptcl )
	{
		ptcl = ptcl->ptclNext;
		TotalParticleCount++;
	}
	
	// return the active count
	return TotalParticleCount;
	
}

void 	Particle_SystemFrame(Particle_System *ps, geFloat DeltaTime)
{
	geVec3d	AnchorDelta;
	//DeltaTime = 0.1f;
	
	// the quick fix to the particle no-draw problem 
	ps->psQuantumSeconds = DeltaTime;
	{
		
		Particle *	ptcl;
		
		ptcl = ps->psParticles;
		while	(ptcl)
		{
			ptcl->ptclTime -= ps->psQuantumSeconds;
			if	(ptcl->ptclTime <= 0.0f)
			{
				Particle *	temp;

				temp = ptcl->ptclNext;
				UnlinkParticle(ps, ptcl);
				DestroyParticle(ps, ptcl);
				ptcl = temp;
				continue;
			}
			else
			{
				
				// locals
				geVec3d		DeltaPos = { 0.0f, 0.0f, 0.0f };
				// apply velocity
				if ( ptcl->ptclFlags & PARTICLE_HASVELOCITY )
				{
					geVec3d_Scale( &( ptcl->ptclVelocity ), ps->psQuantumSeconds, &DeltaPos );
				}
				
				// apply gravity
				if ( ptcl->ptclFlags & PARTICLE_HASGRAVITY )
				{
					// locals
					geVec3d	Gravity;
					
					// make gravity vector
					geVec3d_Scale( &( ptcl->Gravity ), ps->psQuantumSeconds, &Gravity );
					
					// apply gravity to built in velocity and DeltaPos
					geVec3d_Add( &( ptcl->ptclVelocity ), &Gravity, &( ptcl->ptclVelocity ) );
					geVec3d_Add( &DeltaPos, &Gravity, &DeltaPos );
				}
				
				// apply DeltaPos to particle position
				if (( ptcl->ptclFlags & PARTICLE_HASVELOCITY ) || ( ptcl->ptclFlags & PARTICLE_HASGRAVITY ) )
				{
					geVec3d_Add( (geVec3d *)&( ptcl->ptclVertex.X ), &DeltaPos, (geVec3d *)&( ptcl->ptclVertex.X ) );
				}
				
				// make the particle follow its anchor point if it has one
				if ( ptcl->AnchorPoint != (const geVec3d *)NULL )
				{
					geVec3d_Subtract( ptcl->AnchorPoint, &( ptcl->CurrentAnchorPoint ), &AnchorDelta );
					geVec3d_Add( (geVec3d *)&( ptcl->ptclVertex.X ), &AnchorDelta, (geVec3d *)&( ptcl->ptclVertex.X ) );
					geVec3d_Copy( ptcl->AnchorPoint, &( ptcl->CurrentAnchorPoint ) );
				}

				// update scale if needed
				if( ptcl->UseSizeUp ){
					float scaleChange = ptcl->ScaleTo - ptcl->ScaleFrom;
					if( scaleChange < 0.0f ) scaleChange*=-1.0f;
					ptcl->Scale = ptcl->ScaleFrom + scaleChange * ( (ptcl->ptclTotalTime - ptcl->ptclTime) / ptcl->ptclTotalTime );
				}
				
			}
			
#ifndef	POLYQ
			// set particle alpha
			ptcl->ptclVertex.a = ptcl->Alpha * ( ptcl->ptclTime / ptcl->ptclTotalTime );
			geWorld_AddPolyOnce(ps->psWorld,
				&ptcl->ptclVertex,
				1,
				ptcl->ptclTexture,
				GE_TEXTURED_POINT,
				PARTICLE_RENDERSTYLE,
				ptcl->Scale );
#else
			// set particle alpha
			ptcl->ptclVertex.a = ptcl->Alpha * ( ptcl->ptclTime / ptcl->ptclTotalTime );
			
			if(ptcl->ptclPoly){
				gePoly_SetLVertex(ptcl->ptclPoly, 0, &ptcl->ptclVertex);
			//	if( ptcl->UseSizeUp )
			//		ptcl->ptclPoly->Scale = ptcl->Scale; // isn't there any nicer way to do this???
			}
#endif
			
			ptcl = ptcl->ptclNext;
		}
		
		DeltaTime -= QUANTUMSIZE;
	}
	
	ps->psLastTime += DeltaTime;
}


static	Particle * CreateParticle(Particle_System *ps, geBitmap *Texture, const GE_LVertex *Vert )
{
	Particle *ptcl;
	
	ptcl = (Particle*)malloc(sizeof(Particle));
	if	(!ptcl)
		return ptcl;
	
	memset(ptcl, 0, sizeof(*ptcl));
	
	ptcl->ptclNext = ps->psParticles;
	ps->psParticles = ptcl;
	if(ptcl->ptclNext)
		ptcl->ptclNext->ptclPrev = ptcl;
	ptcl->ptclTexture = Texture;
	ptcl->ptclVertex = *Vert;
	
	return ptcl;
}

// removes all references to an anchor point
geBoolean Particle_SystemRemoveAnchorPoint(Particle_System *ps, geVec3d	*AnchorPoint )
{
	// locals	
	Particle	*ptcl;
	geBoolean	AtLeastOneFound = GE_FALSE;
	
	// eliminate achnor point from all particles in this particle system
	ptcl = ps->psParticles;
	while ( ptcl != NULL )
	{
		if ( ptcl->AnchorPoint == AnchorPoint )
		{
			ptcl->AnchorPoint = (const geVec3d *)NULL;
			AtLeastOneFound = GE_TRUE;
		}
		ptcl = ptcl->ptclNext;
	}
	
	// all done
	return AtLeastOneFound;
}

void 	Particle_SystemAddParticle(
								   Particle_System		*ps,
								   geBitmap	*Texture,
								   const GE_LVertex	*Vert,
								   const geVec3d		*AnchorPoint,
								   geFloat				Time,
								   const geVec3d		*Velocity,
								   float				Scale,
								   float				ScaleTo,
								   geBoolean			DoScale,
								   const geVec3d		*Gravity )
{
	
	// locals
	Particle	*ptcl;
	
	// create a new particle
	ptcl = CreateParticle( ps, Texture, Vert );
	if ( !ptcl )
	{
		return;
	}
	
	// setup gravity
	if ( Gravity != (const geVec3d *)NULL )
	{
		geVec3d_Copy( Gravity, &( ptcl->Gravity ) );
		ptcl->ptclFlags |= PARTICLE_HASGRAVITY;
	}
	
	// setup velocity
	if ( Velocity != (const geVec3d *)NULL )
	{
		geVec3d_Copy( Velocity, &( ptcl->ptclVelocity ) );
		ptcl->ptclFlags |= PARTICLE_HASVELOCITY;
	}
	
	// setup the anchor point
	if ( AnchorPoint != (const geVec3d *)NULL )
	{
		geVec3d_Copy( AnchorPoint, &( ptcl->CurrentAnchorPoint ) );
		ptcl->AnchorPoint = AnchorPoint;
	}
	
	// setup remaining data
	ptcl->Scale = Scale;
	ptcl->ScaleFrom = Scale;
	ptcl->UseSizeUp = DoScale;
	ptcl->ScaleTo = ScaleTo;
	ptcl->ptclTime = Time;
	ptcl->ptclTotalTime = Time;
	ptcl->Alpha = Vert->a;
	
	// add the poly to the world
#ifdef	POLYQ
	ptcl->ptclPoly = geWorld_AddPoly(ps->psWorld,
		&ptcl->ptclVertex,
		1,
		ptcl->ptclTexture,
		GE_TEXTURED_POINT,
		PARTICLE_RENDERSTYLE,
		ptcl->Scale );
#endif
}

void  Particle_SystemReset(Particle_System *ps)
{
	ps->psLastTime = 0.0f;
	Particle_SystemRemoveAll(ps);
}

