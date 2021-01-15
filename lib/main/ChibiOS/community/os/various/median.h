/*
    ChibiOS-Contrib - Copyright (C) 2014...2019 Fabien Poussin

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#ifndef MEDIAN_H_
#define MEDIAN_H_

#include "ch.h"

typedef struct pair_
{
  struct pair_* point; /* Pointers forming list linked in sorted order */
  uint16_t value;      /* Values to sort */
} pair_t;

typedef struct
{
  uint16_t stopper;    /* Smaller than any datum */
  uint16_t size;       /* 3 or more */
  pair_t* buffer;      /* Buffer of nwidth pairs */
  pair_t* datpoint;    /* Pointer into circular buffer of data */
  pair_t small;        /* Chain stopper */
  pair_t big;          /* Pointer to head (largest) of linked list.*/
} median_t;

void median_init(median_t* conf, uint16_t stopper, pair_t* buffer, uint16_t size);
uint16_t median_filter(median_t* conf, uint16_t datum);
uint16_t middle_of_3(uint16_t a, uint16_t b, uint16_t c);

#endif /* MEDIAN_H_ */
