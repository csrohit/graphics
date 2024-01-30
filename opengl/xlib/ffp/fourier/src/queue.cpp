#include "queue.h"

CircularBuffer* createCircularBuffer(int size) {
    CircularBuffer* cb = (CircularBuffer*)malloc(sizeof(CircularBuffer));
    if (cb == NULL) {
        return NULL; // Memory allocation failed
    }

    cb->buffer = (float*)malloc(size * sizeof(int));
    if (cb->buffer == NULL) {
        free(cb);
        return NULL; // Memory allocation failed
    }

    cb->size = size;
    cb->head = 0;
    cb->tail = 0;
    cb->count = 0;

    return cb;
}

void freeCircularBuffer(CircularBuffer* cb) {
    free(cb->buffer);
    free(cb);
}

int isBufferFull(CircularBuffer* cb) {
    return cb->count == cb->size;
}

int isBufferEmpty(CircularBuffer* cb) {
    return cb->count == 0;
}

int writeBuffer(CircularBuffer* cb, float data) {
    // if (isBufferFull(cb)) {
    //     return -1; // Buffer is full, cannot write
    // }

    cb->buffer[cb->head] = data;
    cb->head = (cb->head + 1) % cb->size; // Wrap around if needed
    if(cb->count < cb->size)
    cb->count++;

    return 0; // Successful write
}

float readBuffer(CircularBuffer* cb) {
    if (isBufferEmpty(cb)) {
        return -1; // Buffer is empty, cannot read
    }

    float data = cb->buffer[cb->tail];
    cb->tail = (cb->tail + 1) % cb->size; // Wrap around if needed
    // cb->count--;

    return data;
}
// int main() {
//     int bufferSize = 5;
//     CircularBuffer* cb = createCircularBuffer(bufferSize);
//
//     // Write data to the circular buffer
//     for (int i = 0; i < bufferSize + 2; ++i) {
//         int result = writeBuffer(cb, i);
//         if (result == -1) {
//             printf("Buffer is full! Couldn't write: %d\n", i);
//         }
//     }
//
//     // Read data from the circular buffer
//     while (!isBufferEmpty(cb)) {
//         int data = readBuffer(cb);
//         printf("Read: %d\n", data);
//     }
//
//     freeCircularBuffer(cb);
//     return 0;
// }
