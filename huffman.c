// Huffman Encoding and Decoding
//
// Author: Jamie Holdcroft (z5591952)
//
// Written: 30/03/2025
//
/* Description: huffman.c implements Huffman encoding and decoding. 
Decodes an encoding uses a huffman tree and returns a file with the decoding. 
Several helper functions are provided for building and traversing the trees, 
merging nodes, and managing memory.*/

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "character.h"
#include "Counter.h"
#include "File.h"
#include "huffman.h"

#define TRUE 1
#define FALSE 0
#define UNINITIALISED -1
#define MAX_QUEUE_LENGTH 1000

// A structure for bst nodes that stores a character and its Huffman encoding
struct bst {
	char *character;
	struct bst *left;
	struct bst *right;
	char *encoding;
};

/* A structure for elements in the queue that is used when traversing the 
Huffman tree */
struct nodeQueue {
    struct huffmanTree *node;
	char *path; 
};

void decode(struct huffmanTree *tree, char *encoding, char *outputFilename);

struct huffmanTree *createHuffmanTree(char *inputFilename);

Counter createCounterBst(char *inputFilename);

static void findTwoMins(struct huffmanTree **nodes, int size, int *min1, 
int *min2);

static void mergeTwoMins(struct huffmanTree **nodes, int *min1, int *min2);

char *encode(struct huffmanTree *tree, char *inputFilename);

static struct bst *insertBst(struct bst *node, char *character, 
char *currentPath);

struct bst *bfsTraversal(struct huffmanTree *tree);

void freeBst(struct bst *node);

static struct bst *findNode(struct bst *node, char *character);

/* Takes in a huffman tree and a string of 0's and 1's and traverses the
huffman tree accordingly until it finds a leaf node. It then writes the 
character stored in the character field of that leaf node into an output file, 
and repeats until it reaches the null terminator of the encoding string */

void decode(struct huffmanTree *tree, char *encoding, char *outputFilename) {

	struct huffmanTree *current = tree;
	File outputFile = FileOpenToWrite(outputFilename);

	/* Iterates through each character in the encoding string and traverses the 
	tree until a leaf node is reached */
	for (int i = 0; encoding[i] != '\0'; i++) {

		if (encoding[i] == '0') current = current->left;
		else if (encoding[i] == '1') current = current->right;

		if (current->left == NULL && current->right == NULL) {
			FileWrite(outputFile, current->character);
			current = tree;
		}
	}

	FileClose(outputFile);
}

/* Takes in a file of characters and uses the Counter ADT to create an array
of all the characters and their frequencies. Then converts the elements of that 
array into tree nodes and constructs a huffman tree for the input file wherein 
the highest frequency characters are closer to the root node for efficient 
compression */

struct huffmanTree *createHuffmanTree(char *inputFilename) {

	Counter c = createCounterBst(inputFilename);

	// Creates an array of items containing each character and its frequency
	int numItems;
	struct item *items = CounterItems(c, &numItems);
	if (numItems == 0) return NULL;

	/* Converts the array of items into an array of pointers to Huffman tree
	nodes */
	struct huffmanTree **nodes = malloc(numItems * sizeof(struct huffmanTree *));

	for (int i = 0; i < numItems; i++) {
    	nodes[i] = malloc(sizeof(struct huffmanTree));
		nodes[i]->character = malloc(strlen(items[i].character) + 1);
		strcpy(nodes[i]->character, items[i].character);
    	nodes[i]->freq = items[i].freq;
    	nodes[i]->left = NULL;
    	nodes[i]->right = NULL;
	}

	free(items); 

	/* Repeatedly merges the smallest two nodes in the nodes array 
	until the entire Huffman tree is created and only the root node 
	remains non-NULL */

	for (int i = 1; i < numItems; i++) {
		int min1;
		int min2;
		findTwoMins(nodes, numItems, &min1, &min2);
		mergeTwoMins(nodes, &min1, &min2);
	}
	
	// Locates the non-NULL pointer to the root node of the Huffman tree 
	struct huffmanTree *huffmanTree = NULL;
	for (int i = 0; i < numItems; i++) {
		if (nodes[i] != NULL) {
			huffmanTree = nodes[i];
			break;
		}
	}
	
	free(nodes);
	CounterFree(c);
	return huffmanTree;
}

/* Creates a 'Counter' data structure which functions as a bst that holds 
information about all of the characters in the input file and their frequency. 
This acts as a necessary intermediate data structure between the input file 
and the desired items array.*/

Counter createCounterBst(char *inputFilename) {

	File inputFile = FileOpenToRead(inputFilename);
	Counter c = CounterNew();
	char character[MAX_CHARACTER_LEN + 1];

	// Reads the entire file and iteratively adds characters to the Counter bst
	while (FileReadCharacter(inputFile, character) == true) CounterAdd(c, character);

	FileClose(inputFile);
	return c;
}

/* A helper function that finds the two minimum elements within the items array
at each time that it is called. After locating, it modidies the min1 and min2
pointers. */

static void findTwoMins(struct huffmanTree **nodes, int size, int *min1, 
int *min2) {

    *min1 = UNINITIALISED;
    *min2 = UNINITIALISED;
    
    for (int i = 0; i < size; i++) {

        if (nodes[i] == NULL) continue;

		// Update min1 and min2 based on the node's frequencies
        if (*min1 == UNINITIALISED || nodes[i]->freq < nodes[*min1]->freq) {
            *min2 = *min1;
            *min1 = i;
        } else if (*min2 == UNINITIALISED || nodes[i]->freq < nodes[*min2]->freq) {
            *min2 = i;
        }
    }
}

/* Merges the two minimum nodes that is it given into a tree, returns the
root node of that tree into the array of remaining nodes. Then sets the second
min's value to NULL in the array to effectively merge the two mins into a single
returned node whose frequency is the sum of all previously merged nodes */

static void mergeTwoMins(struct huffmanTree **nodes, int *min1, int *min2) {

	// Creates a new node whose children are the two min nodes
	struct huffmanTree *new_node = malloc(sizeof(struct huffmanTree));
	new_node->freq = nodes[*min1]->freq + nodes[*min2]->freq;
	new_node->character = NULL;
	new_node->left = nodes[*min1];
	new_node->right = nodes[*min2];

	// Replace the node at min1 with the new root node of the two merged nodes
	nodes[*min1] = new_node;

	// Sets the second node to NULL as it has been merged already
	nodes[*min2] = NULL;
}

/* Given a huffman tree for a particular file and a file consisting of 
characters, encode essentially converts all the relevant information within
the huffman tree into a bst. It then reads the input file character by 
character and creates a string of the encodings of every character in the 
input file. It returns the string which thus, contains the encoded file. */

char *encode(struct huffmanTree *tree, char *inputFilename) {

    if (tree == NULL || inputFilename == NULL) return NULL;

	// Creates an efficient bst from the Huffman tree with the root node 'root'
	struct bst *root = bfsTraversal(tree);
    File inputFile = FileOpenToRead(inputFilename);

	// Initialise the returned result string
	size_t capacity = 1024;
    size_t length = 0;
    char *result = malloc(capacity);
    result[0] = '\0';

    char character[MAX_CHARACTER_LEN + 1];

	/* Read each character from the input file and append its encoding to the 
	result string, reallocating more memory if needed */
    while (FileReadCharacter(inputFile, character)) {

        struct bst *node = findNode(root, character);
		char *code = node->encoding;
		size_t codeLength = strlen(code);

		if (length + codeLength + 1 > capacity) {
			capacity = (length + codeLength + 1) * 2;	
			char *temp = realloc(result, capacity);
			result = temp;
		}

		strcpy(result + length, code);
        length += codeLength;
	}

	freeBst(root);
    FileClose(inputFile);

    return result;
}

/* Given a root node, a character, and an encoding, it creates a node with
the character and encoding information, travserses the bst from the root node 
and inserts the created node into a binary search tree as a leaf node. The bst 
is used to more efficiently store and retrieve characters and their encodings */

static struct bst *insertBst(struct bst *node, char *character, 
char *currentPath) {
	
	// If tree is empty or a leaf node position is reached, creates a new node
	if (node == NULL) {
        struct bst *new_node = malloc(sizeof(struct bst));
        new_node->character = malloc(strlen(character) + 1); 
        strcpy(new_node->character, character);  
        new_node->encoding = malloc(strlen(currentPath) + 1); 
        strcpy(new_node->encoding, currentPath);  
        new_node->left = NULL;
		new_node->right = NULL; 
        return new_node;
    }

	// Traverses the bst until a valid insertion position is reached
	if (strcmp(character, node->character) < 0) {
		node->left = insertBst(node->left, character, currentPath);
	} else if (strcmp(character, node->character) > 0) {
		node->right = insertBst(node->right, character, currentPath);
	}

	return node;
}

/* Given a huffman tree, a breadth first search algorithm travserses the tree
and calls the insertBst function every time a leaf node is reached to
effectively create a binary search tree out of the huffman tree that allows
fast character finding and encoding retrieval */

struct bst *bfsTraversal(struct huffmanTree *tree) {

	// Initialise a queue for bfs using an array
    struct nodeQueue queue[MAX_QUEUE_LENGTH]; 
    int front = 0;
	int rear = 0;

	// Enqueue the root of the huffman tree with empty string for encoding
	struct nodeQueue temp;
	temp.node = tree;
	temp.path = strdup(""); 
	queue[rear] = temp;
	rear++;

	// Flag for whether or not the root has been initialised
	int isFirstLeaf = FALSE;
	struct bst *root = malloc(sizeof(struct bst));
	root->left = NULL;
	root->right = NULL;

	// Process the queue until all nodes have been inserted into the bst
    while (front < rear) {
        struct nodeQueue current = queue[front++];
        struct huffmanTree *currentNode = current.node;
        char *currentPath = current.path;

		// Check if current Huffman tree node is a leaf node (insertable)
        if (currentNode->left == NULL && currentNode->right == NULL) {
			if (isFirstLeaf == FALSE) {
				root->encoding = malloc(strlen(currentPath) + 1);  
    			strcpy(root->encoding, currentPath);  
    			root->character = malloc(strlen(currentNode->character) + 1); 
    			strcpy(root->character, currentNode->character);  
    			isFirstLeaf = TRUE;
			} else {
				insertBst(root, currentNode->character, currentPath);
			}
        }

		/* If theres a left child creates a new path string by adding '0' 
		to the current path */
        if (currentNode->left != NULL) {
            size_t pathLength = strlen(currentPath) + 2;
			char *newPath = malloc(pathLength);
            strcpy(newPath, currentPath);
            strcat(newPath, "0"); 
			struct nodeQueue temp;
			temp.node = currentNode->left;
			temp.path = newPath;
			queue[rear] = temp;
			rear++;
        }

		/* If theres a left child creates a new path string by adding '1' 
		to the current path */
        if (currentNode->right != NULL) {
            size_t pathLength = strlen(currentPath) + 2;
			char *newPath = malloc(pathLength);
            strcpy(newPath, currentPath);
            strcat(newPath, "1"); 
			struct nodeQueue temp;
			temp.node = currentNode->right;
			temp.path = newPath;
			queue[rear] = temp;
			rear++;
		}

		free(currentPath);
    }

	return root;
}

// Frees all nodes in the bst that is created by the function insertBst

void freeBst(struct bst *node) {

    if (node == NULL) return;
	
    free(node->character);
	free(node->encoding);
	freeBst(node->left);
	freeBst(node->right);
	free(node);
}

/* Given a character that is read from the inputFile, it locates the node that
contains that character in the character field in the created bst. It then 
returns the node to the encode function so that the given character's encoding
can be retrieved from the encoding field */

static struct bst *findNode(struct bst *node, char *character) {

	/* Traverses the bst using the character values to decide traversal path 
	until the character field in the current node is equal to the given 
	character. Then returns that node */ 
	if (strcmp(character, node->character) < 0) return findNode(node->left, character);
	else if (strcmp(character, node->character) > 0) return findNode(node->right, character);
	else return node;
	return NULL;
}
