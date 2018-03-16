/*
    ChibiOS/RT - Copyright (C) 2013-2014 Uladzimir Pylinsky aka barthess

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

#ifndef MEMBENCH_H_
#define MEMBENCH_H_

/*
 *
 */
typedef struct {
  void    *start;
  size_t  size;
} membench_t;

/*
 * all values in B/s
 */
typedef struct {
  uint32_t  memset;
  uint32_t  memcpy;
  uint32_t  memcpy_dma;
  uint32_t  memcmp;
} membench_result_t;

/*
 *
 */
#ifdef __cplusplus
extern "C" {
#endif
  void membench_run(membench_t *dest, const membench_t *src, membench_result_t *ret);
#ifdef __cplusplus
}
#endif

#endif /* MEMBENCH_H_ */
