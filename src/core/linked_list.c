#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include "core/linked_list.h"

void* list_get_at(Node* head, int index) {
    int i = 0;
    Node* current = head;

    while (current != NULL) {
        if (index == i)
            return current->item;
        current = current->next;
        i++;
    }
    return NULL;
}

void list_append(Node** head, Node* new_node) {
    new_node->next = NULL;
    if (*head == NULL) {
        *head = new_node;
        return;
    }

    Node* current = *head;
    while (current->next != NULL)
        current = current->next;
    current->next = new_node;
}

void list_remove_and_free(Node** head, Node* node_to_remove) {
    if (*head == NULL) {
        fprintf(stderr, "error: empty LinkedList\n");
        return;
    }

    Node* current = *head;

    if (current == node_to_remove) {
        *head = current->next;
        free(current);
        return;
    }

    while (current->next != NULL) {
        if (current->next == node_to_remove) {
            Node* to_remove = current->next;
            current->next = to_remove->next;
            free(to_remove);
            return;
        }
        current = current->next;
    }

    fprintf(stderr, "error: node not found in list\n");
}

void list_unlink(Node** head, Node* node_to_unlink) {
    if (*head == NULL) {
        fprintf(stderr, "error: empty LinkedList\n");
        return;
    }

    Node* current = *head;

    if (current == node_to_unlink) {
        *head = current->next;
        node_to_unlink->next = NULL;
        return;
    }

    while (current->next != NULL) {
        if (current->next == node_to_unlink) {
            current->next = node_to_unlink->next;
            node_to_unlink->next = NULL;
            return;
        }
        current = current->next;
    }

    fprintf(stderr, "error: node not found in list\n");
}

int list_count(Node* head) {
    int count = 0;
    Node* node = head;
    while (node != NULL) {
        count++;
        node = node->next;
    }
    return count;
}
