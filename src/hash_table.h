#ifndef __HASH_TABLE_H
#define __HASH_TABLE_H

#define HT_DEFAULT_SIZE 100

typedef struct _HashTable HashTable;

HashTable* HT_CreateDefault();
HashTable* HT_Create(unsigned long size);

void HT_Destroy(HashTable* table);

#define HT_DESTROY(a)	{ HT_Destroy(a); a=0; }

void HT_Add(HashTable *table, int data, char* key);
int HT_Load(HashTable *table, char* fileName);

int HT_Find(HashTable *table, char* key, int* data);

void HT_Remove(HashTable *table, char* key);
void HT_Clear(HashTable *table);

#endif