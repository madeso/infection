#include "hash_table.h"
#include "linked_list.h"

#include "stdlib.h"
#include "string.h"
#include "stdio.h"

#define STRING_LENGTH 80

struct _HashTable{
	LinkedList** table;
	unsigned long size;
	unsigned long elements;
};

long hashcode(char* key){
	// based on: http://www.informatics.susx.ac.uk/courses/dats/notes/html/node114.html
	char c;
	int i=0;
	long sum =0;

	if(!key) return 0;

	do{
		c = key[i];
		sum = 31*sum + c;
		i++;
	}while( c );

	if( sum > 0 )
		return sum;
	return -sum;
}

HashTable* HT_Create(unsigned long size){
	HashTable* res=0;
	unsigned long i;

	if( size <= 1 ){
		printLog("The size of the table should be atleast 2. 100-200 is a recomended, it depends a lot on hpw much data that is required.\n\n");
		return 0;
	}

	res = malloc(sizeof(HashTable) );
	if(! res ) return 0;
	memset(res, 0, sizeof(HashTable) );
	res->size = size;
	res->elements = 0;

	res->table = malloc(sizeof(LinkedList*)*size);
	if(! res->table ) return 0;
	memset(res->table, 0, sizeof(LinkedList*)*size);

	for(i=0; i<size; i++){
		res->table[i] = LL_Create();
		if(! res->table[i] )
		{
			HT_Destroy(res);
			return 0;
		}
	}

	return res;
}

HashTable* HT_CreateDefault(){
	return HT_Create(HT_DEFAULT_SIZE);
}

void HT_Destroy(HashTable* table){
	unsigned long i=0;
	if(! table ) return;

	if( table->table ){
		for(i=0; i< table->size; i++){
			if( table->table[i] ){
				LL_DESTROY( table->table[i] );
			}
		}

		free(table->table);
		table->table=0;
	}

	free( table );
}

#define HT_DESTROY(a)	{ HT_Destroy(a); a=0; }

void HT_Add(HashTable *table, int data, char* key){
	int index=0;
	if(! table ) return;

	index = hashcode(key) % table->size;

	// pay it forward
	LL_Add(table->table[index], data, key);

	table->elements++;
}

int HT_Load(HashTable *table, char* fileName){
	FILE* f=0;
	char key[STRING_LENGTH];
	int data;
	int count=0;

	f = fopen(fileName, "r");
	if(!f) return 0;
	while( !feof(f) ){
		memset(key, 0, sizeof(char)*STRING_LENGTH);
		data = 0;
		fscanf(f, "%s %i", key, &data);
		if( strlen(key) > 0 ){
			HT_Add(table, data, key);
			count++;
		}
	}

	fclose(f);
	{
		char str[400];
		sprintf(str, "\n\n\n\n\n****File count is %i\nElements is %i\n Table size is\n\n\n", count, table->elements, table->size);
		printLog(str);
	}
	return 1;
}

int HT_Find(HashTable *table, char* key, int* data){
	int index=0;
	if(! table ) return 0;

	index = hashcode(key) % table->size;

	// pay it forward
	return LL_Find(table->table[index], key, data);
}

void HT_Remove(HashTable *table, char* key){
	int index=0;
	if(! table ) return;

	index = hashcode(key) % table->size;

	LL_Remove(table->table[index], key);
}

void HT_Clear(HashTable *table){
	unsigned int i=0;
	for( i=0; i<table->size; i++){
		LL_Clear(table->table[i]);
	}
}