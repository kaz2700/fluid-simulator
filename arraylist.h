#include <SDL2/SDL.h>
#include <stddef.h>
#include <stdio.h>
#include <threads.h>
#include "particle.h"

#ifndef arraylist_header
#define arraylist_header

typedef struct Node {
    void* item;
    struct Node* next;
} Node;

void* getFromList(Node* headNode, int index);
void addToList(Node** listHead, Node* newNode);
void removeFromList(Node** listHead, Node* removingNode);
void unlinkFromList(Node** listHead, Node* unlinkingNode);
int getListLength(Node* node);
#endif