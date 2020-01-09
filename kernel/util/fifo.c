#ifndef UTIL_FIFO_C
#define UTIL_FIFO_C

#define FIFO_INITIAL_SIZE 128
#define FIFO_QUEUE_EMPTY -1

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include "../memory.c"

typedef struct fifo_struct {
    int num_elems;
    int max_elems;
    uint8_t* head;
    uint8_t* curr_elem;
} fifo;

fifo* fifo_new() {
    fifo* ret = alloc(sizeof(fifo));
    void* head = alloc(sizeof(uint8_t) * FIFO_INITIAL_SIZE);
    ret->num_elems = 0;
    ret->max_elems = FIFO_INITIAL_SIZE;
    ret->head = head;
    ret->curr_elem = head;
    return ret;
}

void fifo_optimize(fifo* queue) {
    if (queue->head == queue->curr_elem) {
        // resize, not implemented yet (blocked by reallocarray)
    } else {
        // move all items to the beginning of the list
        for(int i = 0; i < queue->num_elems; i++) {
            queue->head[i] = queue->curr_elem[i];
        }
        queue->curr_elem = queue->head;
    }
}

void fifo_enqueue(fifo* queue, uint8_t value) {
    if (queue->num_elems == queue->max_elems) {
        fifo_optimize(queue);
    }
    queue->curr_elem[queue->num_elems] = value;
    queue->num_elems++;
}

int16_t fifo_dequeue(fifo* queue) {
    if (queue->num_elems <= 0) {
        return FIFO_QUEUE_EMPTY;
    }
    uint8_t ret = queue->curr_elem[0];
    queue->curr_elem++;
    queue->num_elems--;
    return ret;
}

#endif //UTIL_FIFO_C