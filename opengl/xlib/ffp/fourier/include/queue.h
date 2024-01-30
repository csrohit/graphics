#ifndef QUEUE_H
#define QUEUE_H

#include <stdio.h>
#include <stdlib.h>

typedef struct {
    float *buffer;    // Buffer array
    int head;       // Index where the next item will be added
    int tail;       // Index where the oldest item is
    int size;       // Maximum number of items in the buffer
    int count;      // Current number of items in the buffer
} CircularBuffer;

CircularBuffer* createCircularBuffer(int size);

void freeCircularBuffer(CircularBuffer* cb);

int isBufferFull(CircularBuffer* cb);

int isBufferEmpty(CircularBuffer* cb);

int writeBuffer(CircularBuffer* cb, float data);

float readBuffer(CircularBuffer* cb);

#endif // !QUEUE_H
