
#include "genesis.h"

#ifndef ENEMY_DEF_H
#define ENEMY_DEF_H

#pragma warning( disable : 4068 )

#pragma GE_Type("Rat.bmp")
typedef struct Inf_Enemy_Rat
{
///////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma GE_Private
	int state;
	int preState; // is this needed?
	int health;

	geXForm3d *XForm;

	geExtBox *ExtBox;

	float counter;
	geActor *Actor;
///////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma GE_Published

	//Position
	geVec3d Position;
#pragma GE_Origin(Position)
#pragma GE_Documentation(Position, "The rat's position")

	//Orientation
	geVec3d Orientation;
#pragma GE_Angles(Orientation)
#pragma GE_Documentation(Orientation, "The rat's orientation")

#pragma warning( default : 4068 )

} Inf_Enemy_Rat;

#endif