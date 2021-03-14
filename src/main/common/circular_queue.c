#include "circular_queue.h"

void circularBufferInit(circularBuffer_t *circular_buffer, uint8_t *buffer, size_t buffer_size,
                          size_t buffer_element_size) {
    circular_buffer->buffer = buffer;
    circular_buffer->bufferSize = buffer_size;
    circular_buffer->elementSize = buffer_element_size;
    circular_buffer->head = 0;
    circular_buffer->tail = 0;
    circular_buffer->size = 0;
}

void circularBufferPushElement(circularBuffer_t *circularBuffer, uint8_t *element) {
    if (circularBufferIsFull(circularBuffer))
        return;

    memcpy(circularBuffer->buffer + circularBuffer->tail, element, circularBuffer->elementSize);

    circularBuffer->tail += circularBuffer->elementSize;
    circularBuffer->tail %= circularBuffer->bufferSize;
    circularBuffer->size += 1;
}

void circularBufferPopHead(circularBuffer_t *circularBuffer, uint8_t *element) {
    memcpy(element, circularBuffer->buffer + circularBuffer->head, circularBuffer->elementSize);

    circularBuffer->head += circularBuffer->elementSize;
    circularBuffer->head %= circularBuffer->bufferSize;
    circularBuffer->size -= 1;
}

uint8_t circularBufferIsFull(circularBuffer_t *circularBuffer) {
    return circularBufferCountElements(circularBuffer) * circularBuffer->elementSize == circularBuffer->bufferSize;
}

uint8_t circularBufferIsEmpty(circularBuffer_t *circularBuffer) {
    return circularBufferCountElements(circularBuffer) == 0;
}

size_t circularBufferCountElements(circularBuffer_t *circularBuffer) {
    return circularBuffer->size;
}