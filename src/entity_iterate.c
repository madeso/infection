#include "genesis.h"
#include "level.h"
#include "entity_iterate.h"
#include "globalGenesis.h"
#include "console.h"
#include "extra_genesis.h"
#include "timefx.h"
#include "log.h"


/*
How the entities works:
Inf_CommandEntity
	If you get too close to it it executes the command that belongs to this entity.
	This can be a load level command, or any other command that can be executed by execute_command()

*/

// initializes the command entity data
void new_commandEntity( geWorld* world)
{
	geEntity_EntitySet	*Set;
	geEntity			*Entity;

	Set = geWorld_GetEntitySet(World, "Inf_CommandEntity");
	if (Set == NULL) return;
	
	// get first entity
	Entity = geEntity_EntitySetGetNextEntity(Set, NULL);

	// wade thru them all
	while( Entity )
	{
		Inf_CommandEntity* h;
		
		// get data
		h = (Inf_CommandEntity *)geEntity_GetUserData(Entity);
		if( !h )
		{
			// failed to convert the data, perhaps the command entity is an old version
			continue;
		}

		// don't bother to actvate it - the reator of the level might want to activate this at a later time
		// h->alive = GE_TRUE; // we want to activate this one

		// get next entity
		Entity = geEntity_EntitySetGetNextEntity(Set, Entity);
	}
}

void new_button(geWorld* world)
{
    geEntity_EntitySet	*Set;
    geEntity			*Entity;

    Set = geWorld_GetEntitySet(world, "Inf_Button");
	if (Set == NULL) return;

    // enumerate buttons
    for (Entity = geEntity_EntitySetGetNextEntity(Set, NULL); 
		Entity; Entity = geEntity_EntitySetGetNextEntity(Set, Entity) )
    {
        Inf_Button *pButton = (Inf_Button*)geEntity_GetUserData(Entity);

		// remember the owner
		geWorld_ModelSetUserData(pButton->model, pButton);
		
	}
}

// iterates the commandEnttitys
void iterate_commandEntity( geVec3d* pos, geWorld* world) 
{
	geEntity_EntitySet	*Set;
	geEntity			*Entity;
	char done=0;

	Set = geWorld_GetEntitySet(World, "Inf_CommandEntity");
	if (Set == NULL) return;
	
	// get first entity
	Entity = geEntity_EntitySetGetNextEntity(Set, NULL);

	// wade thru them all
	while( Entity && !done )
	{
		Inf_CommandEntity* h;
		
		// get data
		h = (Inf_CommandEntity *)geEntity_GetUserData(Entity);
		if( !h )
		{
			continue;
		}

		if( h->alive )
		{
			if( isWithinSquaredRange(pos, &(h->position), h->radiusSquare) )
			{
				execute_command( h->console_command );
				h->alive = GE_FALSE;
				done = 1;
			}
		}

		// get next entity
		if( !done )
			Entity = geEntity_EntitySetGetNextEntity(Set, Entity);
	}
}

void iterate_doors( geVec3d* pos, geWorld* world) 
{
	geFloat				speed = 1.0f;
    geEntity_EntitySet	*Set;
    geEntity			*Entity;
    geMotion			*TempMotion;
    gePath				*Path;
//	geVec3d				TempVec;
	geVec3d				Temppos;

    Set = geWorld_GetEntitySet(world, "Inf_SimpleDoor");
	if (Set == NULL) return;

    // enumerate doors
    for (Entity = geEntity_EntitySetGetNextEntity(Set, NULL); 
		Entity; Entity = geEntity_EntitySetGetNextEntity(Set, Entity) )
    {
        float tStart, tEnd;

        // get individual door data
        Inf_SimpleDoor *pDoor = (Inf_SimpleDoor*)geEntity_GetUserData(Entity);

		// If the model is even partly open we need to see what's behind it.
		if (pDoor->currentPos != 0.0f) geWorld_OpenModel(world, pDoor->model, GE_TRUE);

		// Don't show what's behind.
		else geWorld_OpenModel(world, pDoor->model, GE_FALSE);

		// If we have a door...
		if (pDoor->model)
		{		

			// Create an XForm for the door.
            geXForm3d xfmDest;

			// Get our actor's position.
			Temppos = *pos; //m_XForm.Translation; 

			/*// get forward vector 
			geXForm3d_GetIn(&GetPlayer()->m_XForm, &TempVec);	
			
			// Move
			geVec3d_AddScaled (&Temppos, &TempVec, 0, &Temppos);

			// Move to hand position.
			Temppos.Y += 100.0f;*/								

			// get motion data for world model
            TempMotion = geWorld_ModelGetMotion(pDoor->model);	

			// get animation path
            Path = geMotion_GetPath(TempMotion, 0);	

			// Find out where to start and stop the motion.
			geMotion_GetTimeExtents(TempMotion, &tStart, &tEnd);
			
			// If we passed the end of the door
			if (pDoor->currentPos >= tEnd) 
			{
				// reset it to 0.0f.
				pDoor->currentPos = 0.0f;
			}

			if(! pDoor->enable ) continue;

			if (
				 (geVec3d_DistanceBetween(&pDoor->origin, &Temppos) <= pDoor->distanceToOpen) ||
				 (pDoor->currentPos > tStart)
			   )
			{
				pDoor->currentPos = pDoor->currentPos + speed * TIME * enemyTime; // Move the model's position

				gePath_Sample(Path, pDoor->currentPos, &xfmDest); // Sample the path into xfmDest
				geWorld_SetModelXForm(world, pDoor->model, &xfmDest); // Render the xfmDest xform.
	        }
		}
	}
}

geVec3d* find_commandEntity(geWorld* world, char* name)
{
	geEntity_EntitySet	*Set;
	geEntity			*Entity;
	geVec3d* theOne = 0;

	Set = geWorld_GetEntitySet(World, "Inf_CommandEntity");
	if (Set == NULL) return 0;
	
	// get first entity
	Entity = geEntity_EntitySetGetNextEntity(Set, NULL);

	// wade thru them all
	while( Entity )
	{
		Inf_CommandEntity* h;
		
		// get data
		h = (Inf_CommandEntity *)geEntity_GetUserData(Entity);
		if( !h )
		{
			// failed to convert the data, perhaps the command entity is an old version
			continue;
		}

		if( strcmp(h->name, name) == 0 ){
			theOne = &(h->position);
		}

		// get next entity
		Entity = geEntity_EntitySetGetNextEntity(Set, Entity);
	}
	
	return theOne;
}

// initializes the command entity data
void commandEntity_enableByName( geWorld* world, char* name)
{
	geEntity_EntitySet	*Set;
	geEntity			*Entity;

	Set = geWorld_GetEntitySet(World, "Inf_CommandEntity");
	if (Set == NULL) return;
	
	// get first entity
	Entity = geEntity_EntitySetGetNextEntity(Set, NULL);

	// wade thru them all
	while( Entity )
	{
		Inf_CommandEntity* h;
		
		// get data
		h = (Inf_CommandEntity *)geEntity_GetUserData(Entity);
		if( !h )
		{
			// failed to convert the data, perhaps the command entity is an old version
			continue;
		}

		if( strcmp(h->name, name) == 0 ){
			h->alive = GE_TRUE;
		}

		// get next entity
		Entity = geEntity_EntitySetGetNextEntity(Set, Entity);
	}
}

// initializes the command entity data
void commandEntity_disableByName( geWorld* world, char* name)
{
	geEntity_EntitySet	*Set;
	geEntity			*Entity;

	Set = geWorld_GetEntitySet(World, "Inf_CommandEntity");
	if (Set == NULL) return;
	
	// get first entity
	Entity = geEntity_EntitySetGetNextEntity(Set, NULL);

	// wade thru them all
	while( Entity )
	{
		Inf_CommandEntity* h;
		
		// get data
		h = (Inf_CommandEntity *)geEntity_GetUserData(Entity);
		if( !h )
		{
			// failed to convert the data, perhaps the command entity is an old version
			continue;
		}

		if( strcmp(h->name, name) == 0 ){
			h->alive = GE_FALSE;
		}

		// get next entity
		Entity = geEntity_EntitySetGetNextEntity(Set, Entity);
	}
}

void simpleDoor_enableByName(geWorld* world, char* name)
{
    geEntity_EntitySet	*Set;
    geEntity			*Entity;

    Set = geWorld_GetEntitySet(world, "Inf_SimpleDoor");
	if (Set == NULL) return;

    // enumerate doors
    for (Entity = geEntity_EntitySetGetNextEntity(Set, NULL); 
		Entity; Entity = geEntity_EntitySetGetNextEntity(Set, Entity) )
    {
        Inf_SimpleDoor *pDoor = (Inf_SimpleDoor*)geEntity_GetUserData(Entity);

		if( strcmp(name, pDoor->name) == 0 ){
			pDoor->enable = GE_TRUE;
		}
	}
}
void simpleDoor_disableByName(geWorld* world, char* name)
{
    geEntity_EntitySet	*Set;
    geEntity			*Entity;

    Set = geWorld_GetEntitySet(world, "Inf_SimpleDoor");
	if (Set == NULL) return;

    // enumerate doors
    for (Entity = geEntity_EntitySetGetNextEntity(Set, NULL); 
		Entity; Entity = geEntity_EntitySetGetNextEntity(Set, Entity) )
    {
        Inf_SimpleDoor *pDoor = (Inf_SimpleDoor*)geEntity_GetUserData(Entity);

		if( strcmp(name, pDoor->name) == 0 ){
			pDoor->enable = GE_FALSE;
		}
	}
}

void button_enableByName(geWorld* world, char* name)
{
    geEntity_EntitySet	*Set;
    geEntity			*Entity;

    Set = geWorld_GetEntitySet(world, "Inf_Button");
	if (Set == NULL) return;

    // enumerate buttons
    for (Entity = geEntity_EntitySetGetNextEntity(Set, NULL); 
		Entity; Entity = geEntity_EntitySetGetNextEntity(Set, Entity) )
    {
        Inf_Button *pButton = (Inf_Button*)geEntity_GetUserData(Entity);

		if( strcmp(name, pButton->name) == 0 ){
			pButton->enabled = GE_TRUE;
		}
	}
}
void button_disableByName(geWorld* world, char* name)
{
    geEntity_EntitySet	*Set;
    geEntity			*Entity;

    Set = geWorld_GetEntitySet(world, "Inf_Button");
	if (Set == NULL) return;

    // enumerate buttons
    for (Entity = geEntity_EntitySetGetNextEntity(Set, NULL); 
		Entity; Entity = geEntity_EntitySetGetNextEntity(Set, Entity) )
    {
        Inf_Button *pButton = (Inf_Button*)geEntity_GetUserData(Entity);

		if( strcmp(name, pButton->name) == 0 ){
			pButton->enabled = GE_FALSE;
		}
	}
}

int button_useEntity(geWorld_Model* model){
	void* data = 0;
	Inf_Button *button;
	data = geWorld_ModelGetUserData(model); 

	if( !data ) return ENTITY_ERROR;
	button = (Inf_Button*)data;
	if( !button ) return ENTITY_ERROR;

	if(! button->enabled ) return ENTITY_DISABLED;

	if( button->activated ){
		execute_command(button->command_toDeactivate);
		if( button->useSwitch ){
			button->activated = GE_FALSE;
		}
		if( button->disableAfterUse ){
			button->enabled = GE_FALSE;
		}
	} else {
		execute_command(button->command_toActivate);
		if( button->useSwitch ){
			button->activated = GE_TRUE;
		}
		if( button->disableAfterUse ){
			button->enabled = GE_FALSE;
		}
	}

	return ENTITY_RUN;
}

void new_entities( geWorld* world)
{
	new_commandEntity(world);
	new_button(world);
}

void iterate_entities( geVec3d* pos, geWorld* world)
{
	iterate_commandEntity(pos, world);
	iterate_doors(pos, world);
}

void delete_entities( geWorld* world)
{
}

geVec3d* findPositionByName(geWorld* world, char* name){
	return find_commandEntity(world, name);
	// this function will call other functions like that find position entities
	// and other non-visible entities
}

void entity_enableByName(geWorld* world, char* name){
	commandEntity_enableByName(world, name);
	simpleDoor_enableByName(world, name);
	button_enableByName(world, name);
}

void entity_disableByName(geWorld* world, char* name){
	commandEntity_disableByName(world, name);
	simpleDoor_disableByName(world, name);
	button_disableByName(world, name);
}

int use_entity(geWorld_Model* model){
	int temp=ENTITY_ERROR;

	temp = button_useEntity(model);
	if( temp != ENTITY_ERROR ) return temp;

	return ENTITY_NO_USABLE_ENTETIES;
}

int handle_Button(SaveFile* file, geWorld* world)
{
    geEntity_EntitySet	*Set;
    geEntity			*Entity;

    Set = geWorld_GetEntitySet(world, "Inf_Button");
	if (Set == NULL) return 1;

    // enumerate buttons
    for (Entity = geEntity_EntitySetGetNextEntity(Set, NULL); 
		Entity; Entity = geEntity_EntitySetGetNextEntity(Set, Entity) )
    {
        Inf_Button *pButton = (Inf_Button*)geEntity_GetUserData(Entity);

		INT( pButton->activated, "Failed to handle pButton->activated" );
		INT( pButton->enabled, "Failed to hande pButton->enabled" );
	}

	return 1;
}

int handle_SimpleDoor(SaveFile* file, geWorld* world)
{
    geEntity_EntitySet	*Set;
    geEntity			*Entity;

    Set = geWorld_GetEntitySet(world, "Inf_SimpleDoor");
	if (Set == NULL) return 1;

    // enumerate doors
    for (Entity = geEntity_EntitySetGetNextEntity(Set, NULL); 
		Entity; Entity = geEntity_EntitySetGetNextEntity(Set, Entity) )
    {
        Inf_SimpleDoor *pDoor = (Inf_SimpleDoor*)geEntity_GetUserData(Entity);

		FLOAT( pDoor->currentPos, "Failed to handle pDoor->currentPos" );
		INT( pDoor->enable, "Failed to handle pDoor->enable" );
	}

	return 1;
}

int handle_CommandEntity( SaveFile* file, geWorld* world)
{
	geEntity_EntitySet	*Set;
	geEntity			*Entity;

	Set = geWorld_GetEntitySet(World, "Inf_CommandEntity");
	if (Set == NULL) return 1;
	
	// get first entity
	Entity = geEntity_EntitySetGetNextEntity(Set, NULL);

	// wade thru them all
	while( Entity )
	{
		Inf_CommandEntity* h;
		
		// get data
		h = (Inf_CommandEntity *)geEntity_GetUserData(Entity);
		
		INT(h->alive, "Failed to handle h->alive");

		// get next entity
		Entity = geEntity_EntitySetGetNextEntity(Set, Entity);
	}

	return 1;
}

int handle_entities(SaveFile* file, geWorld* world){
	if(! handle_CommandEntity(file, world) ) return 0;
	if(! handle_SimpleDoor(file, world) ) return 0;
	if(! handle_Button(file, world) ) return 0;
	return 1;
}