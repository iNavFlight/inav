#ifndef INAV_CIRCULAR_QUEUE_H
#define INAV_CIRCULAR_QUEUE_H

#include "stdint.h"
#include "string.h"

typedef struct circularBuffer_s{
    size_t head;
    size_t tail;
    size_t bufferSize;
    uint8_t * buffer;
    size_t elementSize;
    size_t size;
}circularBuffer_t;

void    circularBufferInit(circularBuffer_t * circularBuffer, uint8_t * buffer, size_t bufferSize, size_t bufferElementSize);
void    circularBufferPushElement(circularBuffer_t * circularBuffer, uint8_t * element);
void    circularBufferPopHead(circularBuffer_t * circularBuffer, uint8_t * element);
uint8_t circularBufferIsFull(circularBuffer_t * circularBuffer);
uint8_t circularBufferIsEmpty(circularBuffer_t *circularBuffer);
size_t  circularBufferCountElements(circularBuffer_t * circularBuffer);

#endif //INAV_CIRCULAR_QUEUE_H
