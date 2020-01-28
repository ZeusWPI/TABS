#ifndef LINKED_LIST_C
#define LINKED_LIST_C

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct page_tag {
    size_t size;

    struct page_tag *prev;
    struct page_tag *next;
} page_tag;


page_tag *start;


page_tag *__searchPage(void *data) {
    page_tag *curr_page = start;
    while (curr_page + (sizeof(page_tag)) != data) {
        curr_page = curr_page->next;
    }
    return curr_page;
}

///**
// * @param alignment
// * @param size
// * @return A block aligned with the alignment parameter.
// */
//void* aligned_alloc(int alignment, int size){
//    // TODO
//    return NULL;
//}

void *__alloc(size_t size) {
    page_tag *curr_page = start;

    while (curr_page->next != NULL) {
        page_tag *new_page = ((void *) curr_page) + sizeof(page_tag) + curr_page->size;
        if ((void *) new_page + sizeof(page_tag) + size <= curr_page->next) {

            new_page->size = size;

            curr_page->next->prev = new_page;
            new_page->next = curr_page->next;

            curr_page->next = new_page;
            new_page->prev = curr_page;

            return new_page + 1;
        }
        curr_page = curr_page->next;
    }

    page_tag *new_page = ((void *) curr_page) + sizeof(page_tag) + curr_page->size;
    new_page->size = size;

    curr_page->next = new_page;
    new_page->prev = curr_page;
    return new_page + 1;
}

void __free(void *data) {
    page_tag *data_tag = data - sizeof(page_tag);
    printf("found tag at %p\n", data_tag);

    data_tag->prev->next = data_tag->next;
    data_tag->next->prev = data_tag->prev;
}

void print() {
    printf("==== MEM DUMP ====\n");
    page_tag *curr_page = start;
    int i = 0;
    while (curr_page != NULL) {
        printf("%d: [%p (%zu)] [%p (%zu)]\n",
               i,
               curr_page, sizeof(page_tag),
               curr_page + 1, curr_page->size);

        void *empty_start = (void *) curr_page + sizeof(page_tag) + curr_page->size;
        if (empty_start + sizeof(page_tag) < (void *) curr_page->next) {
            printf("empty_page (%ld, %ld)\n",
                   sizeof(page_tag),
                   (void *) curr_page->next -
                   (empty_start + sizeof(page_tag)));

        } else if (empty_start < (void *) curr_page->next) {
            printf("not enough room (%ld)\n",
                   (void *) curr_page->next - empty_start);
        }
        curr_page = curr_page->next;
        i++;
    }
    printf("-------------------\n\n");
}

void test_allocs() {
    void *ptr0 = __alloc(64);
    void *ptr1 = __alloc(64);
    void *ptr4 = __alloc(64);
    print();

    printf("Free nr 2 -> %p\n", ptr1);
    __free(ptr1);
    print();

    printf("Alloc 32 bytes\n");
    void *ptr2 = __alloc(32);
    print();

    printf("Alloc 32 bytes\n");
    __alloc(32);
    print();

    printf("Alloc 8 bytes\n");
    void *ptr3 = __alloc(8);
    print();

    __free(ptr2);
    print();
    __free(ptr4);
    print();
    __free(ptr3);
    print();

    __free(ptr0);
    print();

    printf("#########\nTry almost fill\n");
    __alloc(160);
    print();
    __alloc(50);
    print();


}

int main() {
    start = malloc(sizeof(size_t) * 1000);
    start->next = NULL;
    start->prev = NULL;
    test_allocs();
}

#endif // LINKED_LIST_C
