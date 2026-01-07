#include <stddef.h>
#include <stdio.h>
#include "particle.h"
#include "artist.h"
#include "arraylist.h"
#include "space_partition.h"

void* getFromList(Node* headNode, int index) {
    int i = 0;
    Node* current = headNode;

    while (current != NULL) {
        if (index == i)
            return current->item;
        current = current->next;
        i++;
    }
    return NULL;
}

void addToList(Node** listHead, Node* newNode) { 
    newNode->next = NULL;
    if (*listHead == NULL) {
        *listHead = newNode;
        return;
    }

    Node* current = *listHead;

    while (current->next != NULL) {
        current = current->next;
    }
    current->next = newNode;
} 

void removeFromList(Node** listHead, Node* removingNode) {
    if (*listHead == NULL) {
        printf("error: empty LinkedList\n");
        return;
    }

    Node* current = *listHead;

    if (current == removingNode) {
        Node* toRemove = *listHead;
        *listHead = toRemove->next;
        free(toRemove);
        printf("removed head\n");
        return;
    }

    int i = 0;

     while (current->next != NULL) {
        if (current->next == removingNode) {
            Node* toRemove = current->next;
            current->next = toRemove->next;
            free(toRemove);
            printf("removed index %d\n", i);
            return;
        }
        current = current->next;
        i++;
    }

    printf("error: index out of range\n");
} 

int getListLength(Node* head) {
    int length = 0;
    Node* node = head;
    while (node != NULL) {
        length++;
        node = node->next;
    }
    return length;
}