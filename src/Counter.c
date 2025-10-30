// Implementation of the Counter ADT

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Counter.h"
#include "character.h"

struct counterNode {
	char character[MAX_CHARACTER_LEN + 1];
	int freq;
	struct counterNode *left;
	struct counterNode *right;
};

struct counter {
	struct counterNode *root;
	int numItems;
};

Counter CounterNew(void) {
	Counter c = malloc(sizeof(*c));
	c->root = NULL;
	c->numItems = 0;
	return c;
}

static void freeNode(struct counterNode *node) {
	if (node == NULL) {
		return;
	}
	freeNode(node->left);
	freeNode(node->right);
	free(node);
}

void CounterFree(Counter c) {
	if (c == NULL) {
		return;
	}
	freeNode(c->root);
	free(c);
}

static struct counterNode *insertNode(struct counterNode *node, char *character, int *numItems) {
	if (node == NULL) {
		struct counterNode *new_node = malloc(sizeof(struct counterNode));
		strcpy(new_node->character, character);
		new_node->left = NULL;
		new_node->right = NULL;
		new_node->freq = 1;
		(*numItems)++;
		return new_node;
	} 

	if (strcmp(character, node->character) < 0) {
		node->left = insertNode(node->left, character, numItems);
	} else if (strcmp(character, node->character) > 0) {
		node->right = insertNode(node->right, character, numItems);
	} else {
		node->freq++;
	}
	
	return node;
}

void CounterAdd(Counter c, char *character) {
	if (c == NULL) {
		return;
	}
    c->root = insertNode(c->root, character, &(c->numItems));
}

int CounterNumItems(Counter c) {
	return c->numItems;
}

int CounterGet(Counter c, char *character) {
	struct counterNode *current = c->root; 
	if (c == NULL || current == NULL) {
		return 0;
	}
	
	while (current != NULL) {
		if (strcmp(character, current->character) < 0) {
			current = current->left;
		} else if (strcmp(character, current->character) > 0) {
			current = current->right;
		} else {
			return current->freq;
		}
	}
	
	return 0;
}

static void arrayOfItems(struct counterNode *node, struct item *items_array, int *index) {
    if (node == NULL) {
		return; 
	} 

    arrayOfItems(node->left, items_array, index); 

    strcpy(items_array[*index].character, node->character);
    items_array[*index].freq = node->freq;
    (*index)++;  

    arrayOfItems(node->right, items_array, index); 
}

struct item *CounterItems(Counter c, int *numItems) {
	if (c == NULL) {
        *numItems = 0;
        return NULL;
    }

    *numItems = c->numItems;
	if (*numItems == 0) {
		return NULL;
	}

    struct item *items_array = malloc((*numItems) * sizeof(struct item));
    int index = 0;
    arrayOfItems(c->root, items_array, &index);
    return items_array;
}

