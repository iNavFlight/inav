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

#include "median.h"

void median_init(median_t* conf, uint16_t stopper, pair_t* buffer, uint16_t size)
{
  conf->stopper = stopper;
  conf->buffer = buffer;
  conf->size = size;
  pair_t small_tmp = {NULL, conf->stopper};
  pair_t big_tmp = {&conf->small, 0};
  conf->datpoint = conf->buffer;                  /* Pointer into circular buffer of data */
  conf->small = small_tmp;                        /* Chain stopper */
  conf->big = big_tmp;                            /* Pointer to head (largest) of linked list.*/
}

uint16_t median_filter(median_t* conf, uint16_t datum)
{
  pair_t *successor;                              /* Pointer to successor of replaced data item */
  pair_t *scan;                                   /* Pointer used to scan down the sorted list */
  pair_t *scanold;                                /* Previous value of scan */
  pair_t *median;                                 /* Pointer to median */
  uint16_t i;

  if (datum == conf->stopper)
  {
    datum = conf->stopper + 1;                    /* No stoppers allowed. */
  }

  if ( (++conf->datpoint - conf->buffer) >= conf->size)
  {
    conf->datpoint = conf->buffer;               /* Increment and wrap data in pointer.*/
  }

  conf->datpoint->value = datum;                 /* Copy in new datum */
  successor = conf->datpoint->point;             /* Save pointer to old value's successor */
  median = &conf->big;                           /* Median initially to first in chain */
  scanold = NULL;                                /* Scanold initially null. */
  scan = &conf->big;                             /* Points to pointer to first (largest) datum in chain */

  /* Handle chain-out of first item in chain as special case */
  if (scan->point == conf->datpoint)
  {
    scan->point = successor;
  }
  scanold = scan;                                     /* Save this pointer and   */
  scan = scan->point ;                                /* step down chain */

  /* Loop through the chain, normal loop exit via break. */
  for (i = 0 ; i < conf->size; ++i)
  {
    /* Handle odd-numbered item in chain  */
    if (scan->point == conf->datpoint)
    {
      scan->point = successor;                      /* Chain out the old datum.*/
    }

    if (scan->value < datum)                        /* If datum is larger than scanned value,*/
    {
      conf->datpoint->point = scanold->point;             /* Chain it in here.  */
      scanold->point = conf->datpoint;                    /* Mark it chained in. */
      datum = conf->stopper;
    };

    /* Step median pointer down chain after doing odd-numbered element */
    median = median->point;                       /* Step median pointer.  */
    if (scan == &conf->small)
    {
        break;                                      /* Break at end of chain  */
    }
    scanold = scan;                               /* Save this pointer and   */
    scan = scan->point;                           /* step down chain */

    /* Handle even-numbered item in chain.  */
    if (scan->point == conf->datpoint)
    {
      scan->point = successor;
    }

    if (scan->value < datum)
    {
      conf->datpoint->point = scanold->point;
      scanold->point = conf->datpoint;
      datum = conf->stopper;
    }

    if (scan == &conf->small)
    {
      break;
    }

    scanold = scan;
    scan = scan->point;
  }
  return median->value;
}

uint16_t middle_of_3(uint16_t a, uint16_t b, uint16_t c)
{
  uint16_t middle;

  if ((a <= b) && (a <= c))
  {
    middle = (b <= c) ? b : c;
  }
  else if ((b <= a) && (b <= c))
  {
    middle = (a <= c) ? a : c;
  }
  else
  {
    middle = (a <= b) ? a : b;
  }
  return middle;
}
