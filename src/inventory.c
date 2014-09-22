#include "inventory.h"
#include "hash_table.h"


HashTable* g_inventory;

void inventory_add(const char* name){
	HT_Add(g_inventory, 1, name);
}

void inventory_remove(const char* name){
	HT_Remove(g_inventory, name);
}

void inventory_clear(){
	HT_Clear(g_inventory);
}

char inventory_create(){
	g_inventory = HT_Create(100);
	return g_inventory != 0;
}

void inventory_delete(){
	if( g_inventory ){
		HT_DESTROY( g_inventory );
	}
}

char inventory_has(const char* filename){
	int data=0;
	return HT_Find(g_inventory, filename, &data) != 0;
}