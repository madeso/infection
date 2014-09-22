/*
entity_iterate.h
Iterates some entities. 
It will iterate the following enteties:
- Inf_CommandEntity

It will also iterate:
- Inf_ActivateEntity (such as buttons and other activateable things)
*/

#ifndef ENTITY_ITERATE_H
#define ENTITY_ITERATE_H

#include "genesis.h"
#include "save_file.h"
handle_entities(SaveFile* file, geWorld* world);

void new_entities( geWorld* world); // initializes the data of the entities
void iterate_entities( geVec3d* pos, geWorld* world); // iterates the enities
void delete_entities( geWorld* world);// deletes the entities
geVec3d* findPositionByName(geWorld* world, char* name);

void entity_enableByName(geWorld* world, char* name);
void entity_disableByName(geWorld* world, char* name);

#define ENTITY_NO_USABLE_ENTETIES 2
#define ENTITY_ERROR 0
#define ENTITY_DISABLED -1
#define ENTITY_RUN 1


int use_entity(geWorld_Model* model);

#endif