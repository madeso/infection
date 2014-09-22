#ifndef __INVENTORY_H
#define __INVENTORY_H

void inventory_add(char* name);
void inventory_remove(char* name);
void inventory_clear();

char inventory_create();
void inventory_delete();

char inventory_has(char* filename);

#endif