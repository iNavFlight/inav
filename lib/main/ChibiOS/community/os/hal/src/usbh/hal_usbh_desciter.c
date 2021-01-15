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

#include "hal.h"

#if HAL_USE_USBH

#include "usbh/defs.h"
#include "usbh/desciter.h"

void cfg_iter_init(generic_iterator_t *icfg, const uint8_t *buff, uint16_t rem) {
	icfg->valid = 0;

	if ((buff[0] < 2) || (rem < 2) || (rem < buff[0])
			|| (buff[0] < USBH_DT_CONFIG_SIZE)
			|| (buff[1] != USBH_DT_CONFIG))
		return;

	if (rem > ((usbh_config_descriptor_t *)buff)->wTotalLength) {
		rem = ((usbh_config_descriptor_t *)buff)->wTotalLength;
	}

	icfg->valid = 1;
	icfg->rem = rem;
	icfg->curr = buff;
}

void if_iter_next(if_iterator_t *iif) {
	const uint8_t *curr = iif->curr;
	uint16_t rem = iif->rem;

	iif->valid = 0;

	if ((curr[0] < 2) || (rem < 2) || (rem < curr[0]))
		return;

	for (;;) {
		rem -= curr[0];
		curr += curr[0];

		if ((curr[0] < 2) || (rem < 2) || (rem < curr[0]))
			return;

		if (curr[1] == USBH_DT_INTERFACE_ASSOCIATION) {
			if (curr[0] < USBH_DT_INTERFACE_ASSOCIATION_SIZE)
				return;

			iif->iad = (usbh_ia_descriptor_t *)curr;

		} else if (curr[1] == USBH_DT_INTERFACE) {
			if (curr[0] < USBH_DT_INTERFACE_SIZE)
				return;

			if (iif->iad) {
				if ((curr[2] < iif->iad->bFirstInterface)
					|| (curr[2] >= (iif->iad->bFirstInterface + iif->iad->bInterfaceCount)))
					iif->iad = 0;
			}
			break;
		}
	}

	iif->valid = 1;
	iif->rem = rem;
	iif->curr = curr;
}

void if_iter_init(if_iterator_t *iif, const generic_iterator_t *icfg) {
	iif->iad = 0;
	iif->curr = icfg->curr;
	iif->rem = icfg->rem;
	if_iter_next(iif);
}

void ep_iter_next(generic_iterator_t *iep) {
	const uint8_t *curr = iep->curr;
	uint16_t rem = iep->rem;

	iep->valid = 0;

	if ((curr[0] < 2) || (rem < 2) || (rem < curr[0]))
		return;

	for (;;) {
		rem -= curr[0];
		curr += curr[0];

		if ((curr[0] < 2) || (rem < 2) || (rem < curr[0]))
			return;

		if ((curr[1] == USBH_DT_INTERFACE_ASSOCIATION)
				|| (curr[1] == USBH_DT_INTERFACE)
				|| (curr[1] == USBH_DT_CONFIG)) {
			return;
		} else if (curr[1] == USBH_DT_ENDPOINT) {
			if (curr[0] < USBH_DT_ENDPOINT_SIZE)
				return;

			break;
		}
	}

	iep->valid = 1;
	iep->rem = rem;
	iep->curr = curr;
}

void ep_iter_init(generic_iterator_t *iep, const if_iterator_t *iif) {
	iep->curr = iif->curr;
	iep->rem = iif->rem;
	ep_iter_next(iep);
}

void cs_iter_next(generic_iterator_t *ics) {
	const uint8_t *curr = ics->curr;
	uint16_t rem = ics->rem;

	ics->valid = 0;

	if ((curr[0] < 2) || (rem < 2) || (rem < curr[0]))
		return;

	rem -= curr[0];
	curr += curr[0];

	if ((curr[0] < 2) || (rem < 2) || (rem < curr[0]))
		return;

	if ((curr[1] == USBH_DT_INTERFACE_ASSOCIATION)
			|| (curr[1] == USBH_DT_INTERFACE)
			|| (curr[1] == USBH_DT_CONFIG)
			|| (curr[1] == USBH_DT_ENDPOINT)) {
		return;
	}

	ics->valid = 1;
	ics->rem = rem;
	ics->curr = curr;
}

void cs_iter_init(generic_iterator_t *ics, const generic_iterator_t *iter) {
	ics->curr = iter->curr;
	ics->rem = iter->rem;
	cs_iter_next(ics);
}

#endif
