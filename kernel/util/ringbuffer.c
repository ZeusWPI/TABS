#ifndef RINGBUFFER_C
#define RINGBUFFER_C

#include <stdbool.h>
#include "../memory.c"

/*
 * Data layout:       ↓head
 * [ 4, undef, undef, 1, 2, 3 ]  (1 is first added etc.)
 * So enqueue adds after 4 and dequeue removes 1.
 */
struct ringbuffer {
	int head;
	int size;
	void** buffer;
	int buffer_n;
};

void rbfr_clear(struct ringbuffer* this) {
	this->size = 0;
	this->head = 0;
}

struct ringbuffer* rbfr_create(const int capacity) {
	struct ringbuffer* const this = alloc(sizeof (struct ringbuffer));

	this->buffer = alloc(capacity * sizeof (void*));
	this->buffer_n = capacity;
	rbfr_clear(this);

	return this;
}

void rbfr_destroy(struct ringbuffer* this) {
	free(this->buffer);
	free(this);
}

int rbfr_size(const struct ringbuffer* this) {
	return this->size;
}

int rbfr_capacity(const struct ringbuffer* this) {
	return this->buffer_n;
}

void rbfr_enqueue(struct ringbuffer* this, void* element) {
	if (this->buffer_n == 0) {
		return;
	}

	int index = this->head + this->size;
	if (index >= this->buffer_n) {
		index -= this->buffer_n;
	}
	this->buffer[index] = element;
	if (this->size < this->buffer_n) {
		this->size++;
	} else {
		this->head++;
	}
	return;
}

bool rbfr_peek(const struct ringbuffer* this, void** element) {
	if (this->size == 0) {
		return false;
	}
	*element = this->buffer[this->head];
	return true;
}

bool rbfr_dequeue(struct ringbuffer* this, void** element) {
	if (!rbfr_peek(this, element)) {
		return false;
	}
	this->size--;
	this->head++;
	if (this->head >= this->buffer_n) {
		this->head = 0;
	}
	return true;
}

#endif // RINGBUFFER_C
