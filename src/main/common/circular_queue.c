/*
 * This file is part of INAV Project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Alternatively, the contents of this file may be used under the terms
 * of the GNU General Public License Version 3, as described below:
 *
 * This file is free software: you may copy, redistribute and/or modify
 * it under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/.
 */

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

int circularBufferIsFull(circularBuffer_t *circularBuffer) {
    return circularBufferCountElements(circularBuffer) * circularBuffer->elementSize == circularBuffer->bufferSize;
}

int circularBufferIsEmpty(circularBuffer_t *circularBuffer) {
    return circularBuffer==NULL || circularBufferCountElements(circularBuffer) == 0;
}

size_t circularBufferCountElements(circularBuffer_t *circularBuffer) {
    return circularBuffer->size;
}