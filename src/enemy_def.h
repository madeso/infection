
#include "genesis.h"
#include "enemies.h"

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


} Inf_Enemy_Rat;









#pragma GE_Type("Zombie.bmp")
typedef struct Inf_Enemy_Zombie
{
///////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma GE_Private
	int state;
	geXForm3d *XForm;
	geExtBox *ExtBox;
	float counter;
	geActor *Actor;
	Monster *closest;
///////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma GE_Published

	int Health;
#pragma GE_Origin(Health)
#pragma GE_Documentation(Health, "The zombies health, if lower than 0 a random number will be generated.")
#pragma GE_DefaultValue(Health, "100")

	int Resurections;
#pragma GE_Origin(Resurections)
#pragma GE_Documentation(Resurections, "How many times the zombie will be resurected,´if below zero a random number will be generated")
#pragma GE_DefaultValue(Resurections, "1")

	//Position
	geVec3d Position;
#pragma GE_Origin(Position)
#pragma GE_Documentation(Position, "The rat's position")

	//Orientation
	geVec3d Orientation;
#pragma GE_Angles(Orientation)
#pragma GE_Documentation(Orientation, "The rat's orientation")

} Inf_Enemy_Zombie;













#pragma warning( default : 4068 )
#endif