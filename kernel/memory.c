#ifndef LINKED_LIST_C
#define LINKED_LIST_C


#include <stddef.h>

#include "terminal.c"
#include "util/printer.c"

#define MEMORY_START 0x200000
//#define MEMORY_END 0x300000

//size_t total_memory = MEMORY_END - MEMORY_START;

typedef struct page_tag {
    size_t size;

    struct page_tag *prev;
    struct page_tag *next;
} page_tag;


page_tag *start = (void *) MEMORY_START;;

void *alloc(size_t size) {
    page_tag *curr_page = start;

    while (curr_page->next != NULL) {
        page_tag *new_page = ((void *) curr_page) + sizeof(page_tag) + curr_page->size;
        if ((void *) new_page + sizeof(page_tag) + size <= (void *) curr_page->next) {

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
    new_page->next = NULL;
    return new_page + 1;
}

void free(void *data) {
    page_tag *data_tag = data - sizeof(page_tag);

    data_tag->prev->next = data_tag->next;
    data_tag->next->prev = data_tag->prev;
}

int amount_of_digits(int x) {
    if (x == 0) return 1;
    int n = 0;
    while (x != 0) {
        n += 1;
        x /= 10;
    }
    return n;
}

void print_memory() {
    print("____________________\n");
    print(" ===  MEM DUMP  === \n");
    page_tag *curr_page = start;
    int i = 0;
    bool left = true;
    while (curr_page != NULL) {
        char padding[4] = {' ', ' ', ' ', ' '};
        padding[4 - amount_of_digits(curr_page->size)] = '\0';
        print("%d: [%x (%d)] [%x (%s%d)]",
              i,
              curr_page, sizeof(page_tag),
              (curr_page + 1), padding, curr_page->size);

        if (left) {
            print(" | ");
        } else {
            print("\n");
        }
        left = !left;

        bool empty_print = false;
        void *empty_start = (void *) curr_page + sizeof(page_tag) + curr_page->size;
        if (empty_start + sizeof(page_tag) < (void *) curr_page->next) {
            print("_: empty page (%d, %d)",
                  sizeof(page_tag),
                  (void *) curr_page->next -
                  (empty_start + sizeof(page_tag)));
            empty_print = true;
        } else if (empty_start < (void *) curr_page->next) {
            print("_: not enough room (%d)",
                  (void *) curr_page->next - empty_start);
            empty_print = true;
        }
        if (empty_print) {
            if (left) {
                print(" | ");
            } else {
                print("\n");
            }
            left = !left;
        }


        curr_page = curr_page->next;
        i += 1;
    }
    if (!left) {
        print("\n");
    }
    print("____________________\n");
}

int command_mem_dump(char *string) {
    print_memory();
}

//
//void test_allocs() {
//    void *ptr0 = __alloc(64);
//    void *ptr1 = __alloc(64);
//    void *ptr4 = __alloc(64);
//    print();
//
//    printf("Free nr 2 -> %p\n", ptr1);
//    __free(ptr1);
//    print();
//
//    printf("Alloc 32 bytes\n");
//    void *ptr2 = __alloc(32);
//    print();
//
//    printf("Alloc 32 bytes\n");
//    __alloc(32);
//    print();
//
//    printf("Alloc 8 bytes\n");
//    void *ptr3 = __alloc(8);
//    print();
//
//    __free(ptr2);
//    print();
//    __free(ptr4);
//    print();
//    __free(ptr3);
//    print();
//
//    __free(ptr0);
//    print();
//
//    printf("#########\nTry almost fill\n");
//    __alloc(160);
//    print();
//    __alloc(50);
//    print();
//
//
//}
//
//int main() {
//    start = malloc(sizeof(size_t) * 1000);
//    start->next = NULL;
//    start->prev = NULL;
//    test_allocs();
//}

#endif // LINKED_LIST_C
