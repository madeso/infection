#ifndef __INVENTORY_H
#define __INVENTORY_H

void inventory_add(const char* name);
void inventory_remove(const char* name);
void inventory_clear();

char inventory_create();
void inventory_delete();

char inventory_has(const char* filename);

#endif