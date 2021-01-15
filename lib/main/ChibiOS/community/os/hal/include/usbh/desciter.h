/*
    ChibiOS - Copyright (C) 2006..2017 Giovanni Di Sirio
              Copyright (C) 2015..2017 Diego Ismirlian, (dismirlian (at) google's mail)

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


#ifndef USBH_DESCITER_H_
#define USBH_DESCITER_H_

#include "hal.h"

#if HAL_USE_USBH

#include "usbh/defs.h"


/* DESCRIPTOR PARSING */
#define _generic_iterator_fields \
	const uint8_t *curr;	\
	uint16_t rem;	\
	bool valid;

typedef struct {
	_generic_iterator_fields
} generic_iterator_t;

typedef struct {
	_generic_iterator_fields
	const usbh_ia_descriptor_t *iad;
} if_iterator_t;

void cfg_iter_init(generic_iterator_t *icfg, const uint8_t *buff, uint16_t rem);
void if_iter_init(if_iterator_t *iif, const generic_iterator_t *icfg);
void ep_iter_init(generic_iterator_t *iep, const if_iterator_t *iif);
void cs_iter_init(generic_iterator_t *ics, const generic_iterator_t *iter);
void if_iter_next(if_iterator_t *iif);
void ep_iter_next(generic_iterator_t *iep);
void cs_iter_next(generic_iterator_t *ics);
static inline const usbh_config_descriptor_t *cfg_get(generic_iterator_t *icfg) {
	return (const usbh_config_descriptor_t *)icfg->curr;
}
static inline const usbh_interface_descriptor_t *if_get(if_iterator_t *iif) {
	return (const usbh_interface_descriptor_t *)iif->curr;
}
static inline const usbh_endpoint_descriptor_t *ep_get(generic_iterator_t *iep) {
	return (const usbh_endpoint_descriptor_t *)iep->curr;
}

#endif

#endif /* USBH_DESCITER_H_ */
