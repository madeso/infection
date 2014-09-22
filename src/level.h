////////////////////////////////
#include "genesis.h"
#ifndef __LEVEL_H
#define __LEVEL_H
#pragma warning( disable:4068 )
////////////////////////////////

#define MAX_TRACE_DIST 300.0f

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma GE_Type("CommandEntity.bmp")
typedef struct _Inf_CommandEntity{

#pragma GE_Published
	geBoolean alive;
#pragma GE_Documentation(alive, "Tells if this should be used or nor - can be enabled or disabled in code")
#pragma GE_DefaultValue(alive, "GE_TRUE")

	char *name;
#pragma GE_Documentation(name, "The name of this entity. Is used when referencing this entity")
#pragma GE_DefaultValue(name, "Inf_CommandEntity")

	char *console_command;
#pragma GE_Documentation(console_command, "The console command to execute when the hero get's in range")
#pragma GE_DefaultValue(console_command, "<null>")

	geVec3d position;
#pragma GE_Origin(position)
#pragma GE_Documentation(position, "The position of the command entity")

	geFloat radiusSquare;
#pragma GE_Documentation(radiusSquare, "The squared radius")
#pragma GE_DefaultValue(radiusSquare, "100.0")
} Inf_CommandEntity;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma GE_Type("PlayerStart.bmp")
typedef struct _Inf_PlayerStart{
#pragma GE_Published

	geVec3d origin;
#pragma GE_Origin(origin)
#pragma GE_Documentation(origin, "The origin for the player to start")

	geBoolean keepWeapons;
#pragma GE_Documentation(keepWeapons, "True if the hero will keep his weapons from the previous, false if not")
#pragma GE_DefaultValue(keepWeapons, "True" )

	geVec3d rotation;
#pragma GE_Documentation(rotation, "The player's direction to face when the level is loaded")
#pragma GE_Angles(rotation)

	geBoolean trace;
#pragma GE_Documentation(trace, "If true traces the hero to the ground, maximum distance is 300 texels")
#pragma GE_DefaultValue(trace, "True" )
} Inf_PlayerStart;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma GE_Type("LevelHeader.bmp")
typedef struct _Inf_LevelHeader{
#pragma GE_Published
	char *name;
#pragma GE_Documentation(name, "The level name to display, as game message")
#pragma GE_DefaultValue(name, "<null>")

	char *author;
#pragma GE_Documentation(author, "The author of the level, displayed in console")
#pragma GE_DefaultValue(author, "<null>")

	char *console_command;
#pragma GE_Documentation(console_command, "The console command to execute at the beginning of the level")
#pragma GE_DefaultValue(console_command, "<null>")

	geBoolean use_fog;
#pragma GE_Documentation(use_fog, "True if level should use fog, false if not")
#pragma GE_DefaultValue(use_fog, "False")

		/*geBoolean use_clip;
#pragma GE_Documentation(use_fog, "True if level should clip, false if not")
#pragma GE_DefaultValue(use_fog, "False")*/

	geFloat fog_distance; // set_fog(0.0f, 30.0f, 40.0f, options.meterfog);
#pragma GE_Documentation(fog_distance, "The distance from the player where the visibility is zero")
#pragma GE_DefaultValue(fog_distance, "15.0f")

	GE_RGBA fog_color;
#pragma GE_Documentation(fog_color, "The fog color")
} Inf_LevelHeader;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

Inf_LevelHeader* global_levelHeader;
void level_enableLevelFog();

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma GE_Type("SimpleDoor.bmp")
typedef struct _Inf_SimpleDoor{
	int state;
#pragma GE_Published

	float			currentPos;
#pragma GE_DefaultValue(currentPos, "0.0")
#pragma GE_Documentation(currentPos, "Time currently at in animation.")

	float			distanceToOpen;
#pragma GE_DefaultValue(distanceToOpen, "150.0")
#pragma GE_Documentation(distanceToOpen, "Max Distance person has to be away from the door to open it.")

	geWorld_Model   *model;
#pragma GE_Documentation(model, "The door model")

	geVec3d			origin;
#pragma GE_Origin(origin)
#pragma GE_Documentation(origin, "The origin")

	char *name;
#pragma GE_Documentation(name, "The name of this entity. Is used when referencing this entity")
#pragma GE_DefaultValue(name, "Inf_SimpleDoor")

	geBoolean enable;
#pragma GE_Documentation(enable, "True if it should react on player movement")
#pragma GE_DefaultValue(enable, "True")

	geBoolean pushable;
#pragma GE_Documentation(pushable, "True if the player has to push/use it to open")
#pragma GE_DefaultValue(pushable, "False")

	geBoolean optimized;
#pragma GE_Documentation(optimized, "True if the door will close when player leaves, False if the door will keep openeing and then close")
#pragma GE_DefaultValue(optimized, "True")
} Inf_SimpleDoor;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma GE_Type("MovingEntity.bmp")
typedef struct _Inf_MovingEntity{
#pragma GE_Published
	float			currentPos;
#pragma GE_DefaultValue(currentPos, "0.0")
#pragma GE_Documentation(currentPos, "Time currently at in animation.")

	geWorld_Model   *model;
#pragma GE_Documentation(model, "The door model")

	geVec3d			origin;
#pragma GE_Origin(origin)
#pragma GE_Documentation(origin, "The origin")

	char *name;
#pragma GE_Documentation(name, "The name of this entity. Is used when referencing this entity")
#pragma GE_DefaultValue(name, "Inf_SimpleDoor")

	geBoolean enable;
#pragma GE_Documentation(enable, "True if it should move")
#pragma GE_DefaultValue(enable, "True")

	geBoolean disableAfterAnim;
#pragma GE_Documentation(enable, "True if disable after one anim")
#pragma GE_DefaultValue(enable, "False")
} Inf_MovingEntity;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma GE_Type("Button.bmp")
typedef struct _Inf_Button{
#pragma GE_Published
	char			*command_toDeactivate;
#pragma GE_DefaultValue(command_toDeactivate, "<null>")
#pragma GE_Documentation(command_toDeactivate, "The command to execute when the button is deactivated")

	char			*command_toActivate;
#pragma GE_DefaultValue(command_toActivate, "<null>")
#pragma GE_Documentation(command_toActivate, "The command to execute when the button is activated")

	geWorld_Model   *model;
#pragma GE_Documentation(model, "The button to use")

	geVec3d			origin;
#pragma GE_Origin(origin)
#pragma GE_Documentation(origin, "The origin")

	char *name;
#pragma GE_Documentation(name, "The name of this entity. Is used when referencing this entity")
#pragma GE_DefaultValue(name, "Inf_Button")

	geBoolean enabled;
#pragma GE_Documentation(enabled, "True if it is enabled")
#pragma GE_DefaultValue(enabled, "True")

	geBoolean activated;
#pragma GE_Documentation(activated, "True if it is activated")
#pragma GE_DefaultValue(activated, "False")

		geBoolean useSwitch;
#pragma GE_Documentation(useSwitch, "True if the player can activate/deactivate the button")
#pragma GE_DefaultValue(useSwitch, "True")

		geBoolean disableAfterUse;
#pragma GE_Documentation(disableAfterUse, "True if the button will be disabled after use")
#pragma GE_DefaultValue(disableAfterUse, "True")
} Inf_Button;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////


/*
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma GE_Type("MovingDoor.bmp")
typedef struct _Inf_MovingDoor{
#pragma GE_Private
	int state; // current state
	float stateTime; // the time to the next state
	geVec3d modelPosition; // the position of the model
#pragma GE_Published

	geWorld_Model* model;
#pragma GE_Documentation(model, "The door model");

	geVec3d position;

	float timeAllowedOpen;
#pragma GE_Documentation(timeAllowedOpen, "The number of seconds the door is opened after the hero has left");
#pragma DefaultValue(timeAllowedOpen, "1.0f")

	float sqDistanceToOpen;
#pragma GE_Documentation(sqDistanceToOpen, "The squared distance between the player and the door where the door will open")
#pragma GE_DefaultValue(sqDistanceToOpen, "2500.0f")
	
	float moveDistance;
#pragma GE_Documentation(moveDistance, "The distance in texels to move")
#pragma GE_DefaultValue(moveDistance, "100.0f")

	float openingSpeed;
#pragma GE_Documentation(openingSpeed, "The opening spped, 1=1second 0.5=2seconds 2=0.5seconds")
#pragma GE_DefaultValue(openingSpeed, "1.0f")

	float closingSpeed;
#pragma GE_Documentation(closingSpeed, "The opening spped, 1=1second 0.5=2seconds 2=0.5seconds")
#pragma GE_Documentation(closingSpeed, "1.0f")

	geVec3d direction;
#pragma GE_Documentation(direction, "The direction the door will move in")

	int damage;
#pragma GE_Documentation(damage, "The damage that the door will inflict if you are colliding with it")
#pragma GE_DefaultValue(damage, "0")

	geBoolean closed;
#pragma GE_Documentation(closed, "True if the door is closed, false if not")
#pragma GE_DefaultValue(closed, "False")
} Inf_MovingDoor;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
*/




/////////////////////////////////
#pragma warning( default:4068 )
#endif
////////////////////////////////