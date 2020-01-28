#ifndef LINKED_LIST_C
#define LINKED_LIST_C

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct ll_node {
    void *data;
    int size;
    bool used;

    struct ll_node *previous;
    struct ll_node *next;
} ll_node;

ll_node *__new_node() {
    ll_node *new_node = malloc(sizeof(ll_node));
    new_node->data = NULL;
    new_node->size = 0;
    new_node->used = false;

    new_node->previous = NULL;
    new_node->next = NULL;
    return new_node;
}

ll_node *__search_ll_node(ll_node *heap_head, void *data) {
    ll_node *curr_node = heap_head;
    while (!curr_node->used || curr_node->data != data) {
        curr_node = curr_node->next;
    }
    return curr_node;
}

void *__alloc(ll_node *ll_head, int size) {
    ll_node *curr_node = ll_head;
    ll_node *new_node = __new_node();
    new_node->used = true;
    new_node->size = size;

    while (curr_node->next != NULL) {
        curr_node = curr_node->next;

        if (!curr_node->used && curr_node->size >= size) {
            curr_node->size -= size;

            // xxx -> <new_node> -> empty block -> yyy

            // link new_node to xxx
            curr_node->previous->next = new_node;
            new_node->previous = curr_node->previous;

            // link new_node to empty_block or yyy
            if (curr_node->size == 0) {
                new_node->next = curr_node->next;
                curr_node->next->previous = new_node;
                free(curr_node);
            } else {
                new_node->next = curr_node;
                curr_node->previous = new_node;
            }

            void *data = new_node->previous->data + new_node->previous->size;
            new_node->data = data;
            return new_node->data;
        }
    }
    new_node->previous = curr_node;
    curr_node->next = new_node;

    void *data = new_node->previous->data + new_node->previous->size;
    new_node->data = data;
    return new_node->data;
}

void __free(ll_node *ll_head, void *data) {
    // Search the corresponding block of memory
    // TODO This can be held in a table for quicker lookup
    ll_node *empty_node = __search_ll_node(ll_head, data);
    empty_node->used = false;

    // Free the node
    if (!empty_node->previous->used) {
        empty_node->size += empty_node->previous->size;

        empty_node->previous->previous->next = empty_node;

        ll_node* previous = empty_node->previous;
        empty_node->previous = empty_node->previous->previous;

        free(previous);
    }

    if (!empty_node->next->used) {
        empty_node->size += empty_node->next->size;

        empty_node->next->next->previous = empty_node;

        ll_node* next = empty_node->next;
        empty_node->next = empty_node->next->next;

        free(next);
    }
}

void print(ll_node *ll_head) {
    printf("==== MEM DUMP ====\n");
    ll_node *curr_node = ll_head;
    while (curr_node != NULL) {
        if (!curr_node->used) {
            printf("empty");
        } else {
            printf("%p", curr_node->data);
        }
        printf(" (%d)\n", curr_node->size);

        curr_node = curr_node->next;
    }
    printf("-------------------\n\n");
}

void test_allocs(){
    ll_node *ll_head = __new_node();
    ll_head->data = malloc(sizeof(size_t) * 1000);
    ll_head->used = true;
    ll_head->size = 1;
    void* ptr0 = __alloc(ll_head, 5);
    void* ptr1 = __alloc(ll_head, 5);
    void* ptr4 = __alloc(ll_head, 5);
    print(ll_head);

    printf("Free nr 3\n");
    __free(ll_head, ptr1);
    print(ll_head);

    printf("Alloc 3 bytes\n");
    void* ptr2 = __alloc(ll_head, 3);
    print(ll_head);

    printf("Alloc 3 bytes\n");
    __alloc(ll_head, 3);
    print(ll_head);

    printf("Alloc 2 bytes\n");
    void* ptr3 = __alloc(ll_head, 2);
    print(ll_head);

    __free(ll_head, ptr2);
    print(ll_head);
    __free(ll_head, ptr4);
    print(ll_head);
    __free(ll_head, ptr3);
    print(ll_head);

    __free(ll_head, ptr0);
    print(ll_head);
}

int main() {
    test_allocs();
}

#endif // LINKED_LIST_C
