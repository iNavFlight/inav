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
int     circularBufferIsFull(circularBuffer_t * circularBuffer);
int     circularBufferIsEmpty(circularBuffer_t *circularBuffer);
size_t  circularBufferCountElements(circularBuffer_t * circularBuffer);

#endif //INAV_CIRCULAR_QUEUE_H
