#ifndef __CONVERSION_H
#define __CONVERSION_H

#include "genesis.h"

void conversation_endGame();
void conversation_iterate(geWorld* world, geVec3d *playerPos, float timePassed);
void startConversation(const char* theLevelFile, const char* fileName, geBoolean stopThisIfPrevious);

#endif