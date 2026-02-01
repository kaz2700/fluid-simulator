#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <stddef.h>

typedef struct Node {
    void* item;
    struct Node* next;
} Node;

void* list_get_at(Node* head, int index);
void list_append(Node** head, Node* new_node);
void list_remove_and_free(Node** head, Node* node_to_remove);
void list_unlink(Node** head, Node* node_to_unlink);
int list_count(Node* head);

#endif
