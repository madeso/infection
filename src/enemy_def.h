
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
	float burningTime;
	int burningDamage;
	geXForm3d *XForm;
	geExtBox *ExtBox;
	float counter;
	geActor *Actor;
	int Health;
	Monster *closest;
///////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma GE_Published

	int Type;
#pragma GE_Documentation(Type, "The zombie type. 0=random 1=weak 2=standard 3=tough")
#pragma GE_DefaultValue(Type, "2")

	int Resurections;
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

	int Look;
#pragma GE_Documentation(Look, "Current look of the zombie")
#pragma GE_DefaultValue(Look, "1")
} Inf_Enemy_Zombie;


#pragma GE_Type("Civilian.bmp")
typedef struct Inf_Enemy_Civilian
{
///////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma GE_Private
	int state;
	float burningTime;
	int burningDamage;
	geXForm3d *XForm;
	geExtBox *ExtBox;
	float counter;
	geActor *Actor;
	Monster *closest;
	int Health;
	int time;
	int timeUntilScream;
	float afraidTime;
///////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma GE_Published

	int Value;
#pragma GE_Documentation(Value, "The private value that the civilian can use")
#pragma GE_DefaultValue(Value, "1")

	int Look;
#pragma GE_Documentation(Look, "Current look of the civilian")
#pragma GE_DefaultValue(Look, "1")

	int Give;
#pragma GE_Documentation(Give, "What the civilian will give the player, 0=nothing other=stuff")
#pragma GE_DefaultValue(Give, "0")

	int ai;
#pragma GE_Documentation(ai, "Current ai of the civilian, 0=standing, 1=walking around")
#pragma GE_DefaultValue(ai, "1")

	//Position
	geVec3d Position;
#pragma GE_Origin(Position)
#pragma GE_Documentation(Position, "The civilian's position")

	//Orientation
	geVec3d Orientation;
#pragma GE_Angles(Orientation)
#pragma GE_Documentation(Orientation, "The civilian's orientation")

	char *cmdKill;
#pragma GE_Documentation(cmdKill, "Command to execute when the civilian gets killed")
#pragma GE_DefaultValue(cmdKill, "")

	char *cmdUse;
#pragma GE_Documentation(cmdUse, "Command to execute when the hero uses this civilian")
#pragma GE_DefaultValue(cmdUse, "")

	char *name;
#pragma GE_Documentation(name, "The name of this entity used in code-reference")
#pragma GE_DefaultValue(name, "")

	char *displayName;
#pragma GE_Documentation(displayName, "The civilian's name")
#pragma GE_DefaultValue(displayName, "")

	int numberOfGives;
#pragma GE_Documentation(numberOfGives, "Number of object the civ has to give")
#pragma GE_DefaultValue(numberOfGives, "1")

	geBoolean execute_command;
#pragma GE_Documentation(execute_command, "True if to is to executed command")
#pragma GE_DefaultValue(execute_command, "True")

	geBoolean infinite_commands;
#pragma GE_Documentation(infinite_commands, "True if the command will disable, false if not")
#pragma GE_DefaultValue(infinite_commands, "False")
} Inf_Enemy_Civilian;






#pragma GE_Type("Soldier.bmp")
typedef struct Inf_Enemy_Soldier
{
///////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma GE_Private
	// burning
	float burningTime;
	int burningDamage;
	// alive keeping
	int health;
	int armor;
	// ai
	int state;
	float counter;
	Monster *closest;
	// animation
	geXForm3d *XForm;
	geExtBox *ExtBox;
	geActor *Actor;
	int time;
	int timeUntilShout;
	float enemyShout;
///////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma GE_Published

	int Value;
#pragma GE_Documentation(Value, "The private value that the civilian can use")
#pragma GE_DefaultValue(Value, "1")

	int Look;
#pragma GE_Documentation(Look, "Current look of the civilian")
#pragma GE_DefaultValue(Look, "1")

	int Give;
#pragma GE_Documentation(Give, "What the civilian will give the player, 0=nothing other=stuff")
#pragma GE_DefaultValue(Give, "0")

	int ai;
#pragma GE_Documentation(ai, "Current ai of the civilian, 0=standing, 1=walking around")
#pragma GE_DefaultValue(ai, "1")

	//Position
	geVec3d Position;
#pragma GE_Origin(Position)
#pragma GE_Documentation(Position, "The civilian's position")

	//Orientation
	geVec3d Orientation;
#pragma GE_Angles(Orientation)
#pragma GE_Documentation(Orientation, "The civilian's orientation")

	char *cmdKill;
#pragma GE_Documentation(cmdKill, "Command to execute when the civilian gets killed")
#pragma GE_DefaultValue(cmdKill, "")

	char *cmdUse;
#pragma GE_Documentation(cmdUse, "Command to execute when the hero uses this civilian")
#pragma GE_DefaultValue(cmdUse, "")

	char *name;
#pragma GE_Documentation(name, "The name of this entity used in code-reference")
#pragma GE_DefaultValue(name, "")

	char *displayName;
#pragma GE_Documentation(displayName, "The civilian's name")
#pragma GE_DefaultValue(displayName, "")

	int numberOfGives;
#pragma GE_Documentation(numberOfGives, "Number of object the civ has to give")
#pragma GE_DefaultValue(numberOfGives, "1")

	geBoolean execute_command;
#pragma GE_Documentation(execute_command, "True if to is to executed command")
#pragma GE_DefaultValue(execute_command, "True")

	geBoolean give_ammo;
#pragma GE_Documentation(infinite_commands, "True if soldier can give some ammo")
#pragma GE_DefaultValue(infinite_commands, "False")
} Inf_Enemy_Soldier;



#pragma warning( default : 4068 )
#endif