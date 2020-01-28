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


page_tag *__searchPage(void *data) {
    page_tag *curr_page = start;
    while (curr_page + (sizeof(page_tag)) != data) {
        curr_page = curr_page->next;
    }
    return curr_page;
}

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

void print_memory() {
    terminal_write("==== MEM DUMP ====\n", 19);
    page_tag *curr_page = start;
    int i = 0;
    while (curr_page != NULL) {
        print("%d: [%x (%d)] [%x (%d)]\n",
                i,
                curr_page, sizeof(page_tag),
                (curr_page + 1), curr_page->size);
//
//        void *empty_start = (void *) curr_page + sizeof(page_tag) + curr_page->size;
//        if (empty_start + sizeof(page_tag) < (void *) curr_page->next) {
//            terminal_writestring("_: empty_page (");
//            terminal_writeint(sizeof(page_tag), 10);
//            terminal_writestring(", ");
//            terminal_writeint(
//                    (void *) curr_page->next -
//                    (empty_start + sizeof(page_tag)), 10);
//            terminal_writestring(")\n");
////            printf("empty_page (%ld, %ld)\n",
////                   sizeof(page_tag),
////                   (void *) curr_page->next -
////                   (empty_start + sizeof(page_tag)));
//        } else if (empty_start < (void *) curr_page->next) {
//            terminal_writestring("_: not enough room (");
//            terminal_writeint((void *) curr_page->next - empty_start, 10);
//            terminal_writestring(")\n");
////            printf("not enough room (%ld)\n",
////                   (void *) curr_page->next - empty_start);
//        }
        curr_page = curr_page->next;
        i += 1;
    }
    terminal_write("------------------\n\n", 20);
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
