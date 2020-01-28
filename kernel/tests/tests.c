#ifndef TESTS_C
#define TESTS_C

#define TESTS_ENABLED
#ifdef TESTS_ENABLED


#define mu_assert(message, test) do { if (!(test)) return message; } while (0)
#define mu_run_test(test) do { char *message = test(); tests_run++; \
                                if (message) return message; } while (0)
extern int tests_run;

#include "../memory.c"
#include <stddef.h>


int tests_run = 0;


static char *test_memory() {

    ll_node *ll_head = __new_node();
    ll_head->data = malloc(sizeof(size_t) * 5000);
    ll_head->used = true;
    ll_head->size = 1;
    void *ptr0 = __alloc(ll_head, 5);
    void *ptr1 = __alloc(ll_head, 5);
    void *ptr4 = __alloc(ll_head, 5);
    print(ll_head);
    mu_assert("Error: chain should be complete at 1", ll_head->next != NULL);
    mu_assert("Error: chain should be complete at 2", ll_head->next->next != NULL);
    mu_assert("Error: chain should be complete at 3", ll_head->next->next->next != NULL);
    mu_assert("Error: chain should be complete at 4", ll_head->next->next->next->next == NULL);

    ll_node *current = ll_head;
    while (current->next != NULL) {
        current = current->next;

        mu_assert("Error: block should have size 5", current->size == 5);
        mu_assert("Error: block should be in use", current->used == true);
    }

    __free(ll_head, ptr1);
    mu_assert("", ll_head->next->used == true);
    mu_assert("", ll_head->next->next->used == false);
    mu_assert("", ll_head->next->next->next->used == true);

    printf("Alloc 3 bytes\n");
    void *ptr2 = __alloc(ll_head, 3);
    print(ll_head);

    printf("Alloc 3 bytes\n");
    __alloc(ll_head, 3);
    print(ll_head);

    printf("Alloc 2 bytes\n");
    void *ptr3 = __alloc(ll_head, 2);
    print(ll_head);

    __free(ll_head, ptr2);
    print(ll_head);
    __free(ll_head, ptr4);
    print(ll_head);
    __free(ll_head, ptr3);
    print(ll_head);

    __free(ll_head, ptr0);
    print(ll_head);
    return 0;
}

static char *all_tests() {
    mu_run_test(test_memory);
    return 0;
}
//
//int main(int argc, char **argv) {
//    char *result = all_tests();
//    if (result != 0) {
//        printf("%s\n", result);
//    } else {
//        printf("ALL TESTS PASSED\n");
//    }
//    printf("Tests run: %d\n", tests_run);
//
//    return result != 0;
//}

#endif // TESTS_ENABLED

#endif // TESTS_C