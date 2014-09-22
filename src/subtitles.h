#include "genesis.h"

#ifndef __SUBTITLE_H
#define __SUBTITLE_H

void subtitle_newGame();
char* subtitle_getString();
int subtitle_getPrintX();
void subtitle_setSubtitle(char* newString);
void subtitle_SetEnable(geBoolean newEnable);
geBoolean render_subTitle();

#endif