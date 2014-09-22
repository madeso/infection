#ifndef __INF_MENY_H
#define __INF_MENY_H

#include "genesis.h"

//#define MENY_TYPE_ONE

geBoolean meny_init();
void meny_update();
void meny_preRedner();
void meny_render();
void meny_delete();
void meny_onChar(char c);

void goBack();
char renderMeny();

#endif