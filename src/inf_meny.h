#ifndef __INF_MENY_H
#define __INF_MENY_H

#include "genesis.h"

#define NO_SHADE

geCamera* meny_getCamera();
geWorld* meny_getWorld();

geBoolean meny_init();
void meny_update();
void meny_preRedner();
void meny_render();
void meny_delete();

void goBack();
char renderMeny();

#endif