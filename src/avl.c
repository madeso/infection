#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include "avl.h"

typedef struct _AVLNode AVLNode;

AVLTree* gtree=0;

struct _AVLNode{
	int data;
	char key[STRING_LENGTH];
	AVLNode* left;
	AVLNode* right;
	AVLNode* parent;
};

struct _AVLTree{
	AVLNode* father;
};

void AVL_balance(AVLNode* node, int traverseLimmit, char balanceChilds);

char str[200];
void Avl_PrintTree(AVLNode* node, int index, char* info){
	int i;
	if( !node ) return;

	sprintf(str, "%s: Key = %s, Data = %i\n", info, node->key, node->data );
	for(i=0; i< index; i++){
		printLog("  ");
	}
	printLog(str);

	Avl_PrintTree(node->left, index+1, "left");
	Avl_PrintTree(node->right, index+1, "right");
}


AVLTree* Avl_Create(){
	AVLTree* tree=0;
	tree = malloc(sizeof(AVLTree));
	if(! tree ) return 0;
	tree->father = 0;
	return tree;
}

void Avl_Destroy_tr(AVLNode* node){
	if( node == 0 ) return;
	Avl_Destroy_tr(node->left); node->left = 0;
	Avl_Destroy_tr(node->right); node->right = 0;
	free(node);
}

void Avl_Destroy(AVLTree* tree){
	Avl_Destroy_tr(tree->father);
	free(tree);
}

void Avl_Node_SetParent(AVLNode* node, AVLNode* parent){
	if( !parent && node ) gtree->father = node;
	if( node ) node->parent = parent;
}

AVLNode* Avl_Restructure(AVLNode *node, char balanceChilds){
	AVLNode* a=0, *b=0, *c=0;
	AVLNode* t0=0, *t1=0, *t2=0, *t3=0;
	AVLNode *parent1=0, *parent2=0;

	//printLog("Restructure\n");

	printLog("\n\nNode is:");
	printLog(node->key);
	printLog("\n");

	printLog("Before restructure\n");
	Avl_PrintTree(gtree->father, 0, "Index");

	parent1 = node->parent;
	if( !parent1 ){
		printLog("Parent 1 doesn't exist\n");
		return node->parent;
	}
	parent2 = parent1->parent;
	if( !parent2 ){
		printLog("Parent 2 doesn't exist\n");
		return node->parent;
	}

	if( parent1->right == node ){
		if( parent2->right == parent1 ){
			printLog("single left rotation (a)\n");
			c = node;
			b = parent1;
			a = parent2;
			t0 = a->left;
			t1 = b->left;
			t2 = c->left;
			t3 = c->right;

			Avl_Node_SetParent(b, a->parent);
		} else {
			printLog("double rotation (d)\n");
			b = node;
			a = parent1;
			c = parent2;
			t0 = a->left;
			t1 = b->left;
			t2 = b->right;
			t3 = c->right;

			Avl_Node_SetParent(b, c->parent);
		}
	} else {
		if( parent2->right == parent1 ){
			printLog("double rotation (c)\n");
			b = node;
			c = parent1;
			a = parent2;
			t0 = a->left;
			t1 = b->left;
			t2 = b->right;
			t3 = c->right;

			Avl_Node_SetParent(b, a->parent);
		} else {
			printLog("single rotation (b)\n");
			a = node;
			b = parent1;
			c = parent2;
			t0 = a->left;
			t1 = a->right;
			t2 = b->right;
			t3 = c->right;

			Avl_Node_SetParent(b, c->parent);
		}
	}

	b->left = a; Avl_Node_SetParent(a, b);
	b->right = c; Avl_Node_SetParent(c, b);
	a->left = t0; Avl_Node_SetParent(t0, a);
	a->right = t1; Avl_Node_SetParent(t1, a);
	c->left = t2; Avl_Node_SetParent(t2, c);
	c->right = t3; Avl_Node_SetParent(t3, c);

	if( balanceChilds ){
		AVL_balance(t0, -1, 0);
		AVL_balance(t1, -1, 0);
		//AVL_balance(t2, 5, 0);
		//AVL_balance(t3, 5, 0);
	}


	printLog("\n\nRestructuring done\n\n");
	Avl_PrintTree(gtree->father, 0, "Index");
	return b;
}

/*
void AVL_basicFind(AVLNode* base, char* key, AVLNode** result, AVLNode** parent){
	AVLNode *current = 0;
	int strcmpResult;

	if( !base ) return;
	if( !result ) return;
	if( !key ) return;

	*result = 0;
	if( parent ) *parent = 0;


	current = base;
	if( parent ) *parent = current;
	while(1){
		if( !current ) return;
		strcmpResult = strcmp(current->key, key);
		if( strcmpResult < 0 ){
			if( parent ) *parent = current;
			// go left
			current = current->left;
			continue;
		} else {
			if( strcmpResult == 0 ) {
				// found
				*result = current;
				return;
			} else {
				// go right
				if( parent ) *parent = current;
				current = current->right;
				continue;
			}
		}
	}
}*/

void AVL_basicFind(AVLNode* base, char* key, AVLNode** result, AVLNode** parent){
	int res;
	AVLNode* next = 0;
	if(! base ) return;
	if(! key ) return;

	res = strcmp(base->key, key);

	//printLog("comparing: k/n ");
	//printLog(key);
	//printLog("/");
	//printLog(base->key);
	//printLog("\n");

	if( res == 0){
		if( parent )
			*parent = base->parent;
		*result = base;

		//printLog("Equal\n");
		return;
	}

	if( res < 0 ){
		next = base->left;
		//printLog("Left\n");
	} else {
		next = base->right;
		//printLog("Right\n");
	}

	if( next == 0 ){
		*result = 0;
		if( parent )
			*parent = base;
		//printLog("Failed\n");
		return;
	}

	AVL_basicFind(next, key, result, parent);
	return;
}

int AVL_getHeight_tr(AVLNode *node){
	int lh, rh;
	if(! node ) return 1;
	lh = AVL_getHeight_tr(node->left);
	rh = AVL_getHeight_tr(node->right);
	if( lh > rh ) return lh+1;
	return rh+1;
}

int AVL_needBalance(AVLNode* node){
	//return 0;
	
	if( !node ) return 0;
	return abs( AVL_getHeight_tr(node->right) - AVL_getHeight_tr(node->left)
		        ) > 1;
}

// is this correct
void AVL_balance(AVLNode* node, int traverseLimmit, char balanceChilds){
	AVLNode *current = node;
	//printLog("Balancing...\n");
	while( current ){
		if( AVL_needBalance(current) ){
			AVLNode* temp = current;

			if( traverseLimmit > 0 ){
				traverseLimmit --;
				if( traverseLimmit == 0 ){
					break;
				}
			}

			current = Avl_Restructure(current, balanceChilds);
			if( current == temp ){
				current = current->parent;

				if( current == current->parent ){
					printLog("Restructering malfunction\n");
				}
			}
		}
		else
			current = current->parent;
		if( current ) {
			//printLog(current->key);
			//printLog(" next\n");

			//Avl_PrintTree(gtree->father, 0 );
		}
	}
}

void Avl_Add(AVLTree* tree, int data, char* key){
	AVLNode* node=0, *result=0, *daddy=0;

	gtree = tree;

	node = malloc(sizeof(AVLNode) );
	if(! node ) return;
	memset(node, 0, sizeof(AVLNode) );
	node->data = data;
	strcpy(node->key, key);

	if( ! (tree->father) )
	{
		tree->father = node;
		return;
	}

	AVL_basicFind(tree->father, key, &result, &daddy);
	if( result ){
		free(node);
		printLog("Already existed\n");
		return; // we found that the there already was one - don't add it
	}
	if( !daddy ) {
		// whoops something went wrong
		printLog("Something went wrong\n");
		free(node);
		return;
	}

	if( strcmp(daddy->key, key) <0 ){
		if( daddy->left ){
			free(node);
			printLog("Bad1\n");
			return; // bad
		}
		daddy->left = node;
		node->parent = daddy;
		AVL_balance(node, -1, 1);
		return;
	} else {
		if( daddy->right ){
			free(node);
			printLog("Bad2\n");
			return; // bad
		}

		daddy->right = node;
		node->parent = daddy;
		AVL_balance(node, -1, 1);
		return;
	}
}

int Avl_Load(AVLTree *tree, char* fileName){
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
			Avl_Add(tree, data, key);
			count++;
		}
	}
	fclose(f);
	{
		char str[400];
		int height = 0;
		height = AVL_getHeight_tr(tree->father);
		sprintf(str, "\n\n\n\n\n****The count is %i, height is %i\n\n", count, height);

		//Avl_PrintTree(tree->father, 0, "father" );

		printLog(str);
	}
	return 1;
}

int Avl_Find(AVLTree *tree, char* search, int* data){
	AVLNode* result=0;
	AVL_basicFind(tree->father, search, &result, 0);
	if( result ){
		*data = result->data;
		//system_message("returning data");
		return result->data;
		//return 1;
	}

	{
		char str[300];
		sprintf(str, "Failed to find texture %s", search);
		system_message(str);
	}
	return -1;
}