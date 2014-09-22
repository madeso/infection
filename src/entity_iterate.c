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

#define TYPE_BUTTON 0
#define TYPE_DOOR 1

typedef struct _EntityModelConnection {
	void* data;
	int type;
} EntityModelConnection;

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

geBoolean new_button(geWorld* world)
{
    geEntity_EntitySet	*Set;
    geEntity			*Entity;
	int fail = 0;

    Set = geWorld_GetEntitySet(world, "Inf_Button");
	if (Set == NULL) return GE_TRUE;

    // enumerate buttons
    for (Entity = geEntity_EntitySetGetNextEntity(Set, NULL); 
		Entity; Entity = geEntity_EntitySetGetNextEntity(Set, Entity) )
    {
        Inf_Button *pButton = (Inf_Button*)geEntity_GetUserData(Entity);
		EntityModelConnection* con;

		con=0;
		con = malloc( sizeof(EntityModelConnection) );

		if(! con ){
			fail = 1;
		} else {
			con->data = pButton;
			con->type = TYPE_BUTTON;
		}
		// remember the owner
		geWorld_ModelSetUserData(pButton->model, con);
	}

	if( fail ) return GE_FALSE;
	return GE_TRUE;
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

void disable_movingEntites(geWorld* world, char* name) {
	geFloat				speed = 1.0f;
    geEntity_EntitySet	*Set;
    geEntity			*Entity;

    Set = geWorld_GetEntitySet(world, "Inf_MovingEntity");
	if (Set == NULL) return;

    for (Entity = geEntity_EntitySetGetNextEntity(Set, NULL); 
		Entity; Entity = geEntity_EntitySetGetNextEntity(Set, Entity) )
    {
        Inf_MovingEntity *pDoor = (Inf_MovingEntity*)geEntity_GetUserData(Entity);

		// If we have a door...
		if (pDoor->model)
		{
			if (strcmp(pDoor->name, name) == 0 ) {
				pDoor->enable = GE_FALSE;
			}
		}
	}
}

void enable_movingEntites(geWorld* world, char* name) {
	geFloat				speed = 1.0f;
    geEntity_EntitySet	*Set;
    geEntity			*Entity;

    Set = geWorld_GetEntitySet(world, "Inf_MovingEntity");
	if (Set == NULL) return;

    for (Entity = geEntity_EntitySetGetNextEntity(Set, NULL); 
		Entity; Entity = geEntity_EntitySetGetNextEntity(Set, Entity) )
    {
        Inf_MovingEntity *pDoor = (Inf_MovingEntity*)geEntity_GetUserData(Entity);

		// If we have a door...
		if (pDoor->model)
		{
			if (strcmp(pDoor->name, name) == 0 ) {
				pDoor->enable = GE_TRUE;
			}
		}
	}
}

// Inf_MovingEntity
void iterate_movingEntites(geWorld* world) {
	geFloat				speed = 1.0f;
    geEntity_EntitySet	*Set;
    geEntity			*Entity;
    geMotion			*TempMotion;
    gePath				*Path;

    Set = geWorld_GetEntitySet(world, "Inf_MovingEntity");
	if (Set == NULL) return;

    for (Entity = geEntity_EntitySetGetNextEntity(Set, NULL); 
		Entity; Entity = geEntity_EntitySetGetNextEntity(Set, Entity) )
    {
        float tStart, tEnd;
        Inf_MovingEntity *pDoor = (Inf_MovingEntity*)geEntity_GetUserData(Entity);

		if(! pDoor->enable ) continue;

		// If we have a door...
		if (pDoor->model)
		{
			// Create an XForm for the door.
            geXForm3d xfmDest;
			geBoolean reachedEnd = GE_FALSE;

			TempMotion = geWorld_ModelGetMotion(pDoor->model);	
            Path = geMotion_GetPath(TempMotion, 0);	
			geMotion_GetTimeExtents(TempMotion, &tStart, &tEnd);
			if (pDoor->currentPos >= tEnd){
				pDoor->currentPos = tEnd;
				reachedEnd = GE_TRUE;
				if( pDoor->disableAfterAnim ) {
					pDoor->enable = GE_FALSE;
				}
			}

			pDoor->currentPos = pDoor->currentPos + speed * TIME * enemyTime; // Move the model's position
			gePath_Sample(Path, pDoor->currentPos, &xfmDest); // Sample the path into xfmDest
			geWorld_SetModelXForm(world, pDoor->model, &xfmDest); // Render the xfmDest xform.

			if( reachedEnd ) {
				pDoor->currentPos = 0.0f;
			}
		}
	}
}

geBoolean new_doors(geWorld* world) 
{
	geEntity_EntitySet	*Set;
    geEntity			*Entity;
	int fail = 0;
    
    Set = geWorld_GetEntitySet(world, "Inf_SimpleDoor");
	if (Set == NULL) return GE_TRUE;

    // enumerate doors
    for (Entity = geEntity_EntitySetGetNextEntity(Set, NULL); 
		Entity; Entity = geEntity_EntitySetGetNextEntity(Set, Entity) )
    {
        Inf_SimpleDoor *pDoor = (Inf_SimpleDoor*)geEntity_GetUserData(Entity);

		if (pDoor->model) {	
			EntityModelConnection* con;
			con = 0;
			con = malloc(sizeof(EntityModelConnection));
			if( con ) {
				con->data = pDoor;
				con->type = TYPE_DOOR;
			} else {
				fail = 1;
			}
			pDoor->state = 0;
			geWorld_ModelSetUserData(pDoor->model, con);
		}
	}

	if( fail ) return GE_FALSE;
	return GE_TRUE;
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

		if(! pDoor->enable ){
			continue;
		}

		// If we have a door...
		if (pDoor->model)
		{		
			geXForm3d xfmDest;
			geBoolean within;
			static float increment;


			// If the model is even partly open we need to see what's behind it.
			if (pDoor->currentPos != 0.0f) geWorld_OpenModel(world, pDoor->model, GE_TRUE);

			// Don't show what's behind.
			else geWorld_OpenModel(world, pDoor->model, GE_FALSE);

		
			// Create an XForm for the door.
			increment = speed * TIME * enemyTime;

			// Get player pos
			Temppos = *pos;
			TempMotion = geWorld_ModelGetMotion(pDoor->model);	
            Path = geMotion_GetPath(TempMotion, 0);	
			geMotion_GetTimeExtents(TempMotion, &tStart, &tEnd);

			within = (geVec3d_DistanceBetween(&pDoor->origin, &Temppos) <= pDoor->distanceToOpen) ;

			if( pDoor->optimized ) {
				if( within ){
					pDoor->currentPos += increment;
					if( pDoor->currentPos > tEnd ) {
						pDoor->currentPos = tEnd;
					}
				}
				else {
					pDoor->currentPos -= increment;
					if( pDoor->currentPos < tStart ) {
						pDoor->currentPos = tStart;
					}
				}
			} else {
				switch( pDoor->state ) {
				case 0:
					if( !pDoor->pushable ) {
						if( within ){
							pDoor->state = 1;
						}
					}
					break;
				case 1:
					pDoor->currentPos += increment;
					if( pDoor->currentPos > tEnd ) {
						pDoor->currentPos = tEnd;
						pDoor->state = 3;
					}
					break;
				case 2:
					pDoor->currentPos -= increment;
					if( pDoor->currentPos < tStart ) {
						pDoor->currentPos = tStart;
						pDoor->state = 0;
					}
					break;
				default:
					if( !within ) {
						pDoor->state = 2;
					}
					break;
				}
			}
			
			gePath_Sample(Path, pDoor->currentPos, &xfmDest); // Sample the path into xfmDest
			geWorld_SetModelXForm(world, pDoor->model, &xfmDest); // Render the xfmDest xform.
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

void delete_buttons(geWorld* world)
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
		void* data;

		data = geWorld_ModelGetUserData(pButton->model);

		if( data ){
			free(data);
			geWorld_ModelSetUserData(pButton->model, 0);
		}
	}
}

void delete_doors(geWorld* world)
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
		void* data;

		data = geWorld_ModelGetUserData(pDoor->model);

		if( data ) {
			free(data);
			geWorld_ModelSetUserData(pDoor->model, 0);
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

int button_useEntity(void* data){
	Inf_Button *button;

	if( !data ) return ENTITY_NO_USABLE_ENTETIES;

	button = (Inf_Button*)data;
	if( !button ) return ENTITY_NO_USABLE_ENTETIES;

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

int door_useEntity(void* data){
	Inf_SimpleDoor *door;

	if( !data ) return ENTITY_NO_USABLE_ENTETIES;
	door = (Inf_SimpleDoor*)data;
	if( !door ) return ENTITY_NO_USABLE_ENTETIES;

	if( door->pushable && !door->optimized ) {
		if( door->state == 0 ) {
			door->state = 1;
		}
		else {
			return ENTITY_DISABLED;
		}
	} else {
		return ENTITY_NO_USABLE_ENTETIES;
	}

	return ENTITY_RUN;
}

geBoolean door_canUseEntity(void* data){
	Inf_SimpleDoor *door;

	if( !data ) return GE_FALSE;
	door = (Inf_SimpleDoor*)data;
	if( !door ) return GE_FALSE;

	if( door->pushable && !door->optimized ) {
		if( door->state == 0 ) {
		}
		else {
			return GE_FALSE;
		}
	} else {
		return GE_FALSE;
	}

	return GE_TRUE;
}

geBoolean button_canUseEntity(void* data){
	Inf_Button *button;

	if( !data ) return GE_FALSE;
	button = (Inf_Button*)data;
	if( !button ) return GE_FALSE;

	if(! button->enabled ) return GE_FALSE;

	return GE_TRUE;
}

geBoolean new_entities( geWorld* world)
{
	geBoolean status = GE_TRUE;

	new_commandEntity(world);
	if(! new_button(world) ){
		status = GE_FALSE;
	}
	if(! new_doors(world) ){
		status = GE_FALSE;
	}

	return status;
}

void iterate_entities( geVec3d* pos, geWorld* world)
{
	iterate_commandEntity(pos, world);
	iterate_doors(pos, world);
	iterate_movingEntites(world);
}

void delete_entities( geWorld* world)
{
	if( !World ) return;
	delete_doors(world);
	delete_buttons(world);
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
	enable_movingEntites(world, name);
}

void entity_disableByName(geWorld* world, char* name){
	commandEntity_disableByName(world, name);
	simpleDoor_disableByName(world, name);
	button_disableByName(world, name);
	disable_movingEntites(world, name);
}

int use_entity(geWorld_Model* model){
	EntityModelConnection* con;

	con = geWorld_ModelGetUserData(model); 

	if( !con ) return ENTITY_NO_USABLE_ENTETIES;

	switch( con->type ){
	case TYPE_BUTTON:
		return button_useEntity(con->data);
	case TYPE_DOOR:
		return door_useEntity(con->data);
	default:
		return ENTITY_NO_USABLE_ENTETIES;
	}
}

geBoolean can_use_entity(geWorld_Model* model){
	EntityModelConnection* con;
	con = geWorld_ModelGetUserData(model); 

	if( !con ) return GE_FALSE;

	switch( con->type ){
	case TYPE_BUTTON:
		return button_canUseEntity(con->data);
	case TYPE_DOOR:
		return door_canUseEntity(con->data);
	default:
		return GE_FALSE;
	}
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
		INT( pDoor->state, "Failed to handle pDoor->state" );
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