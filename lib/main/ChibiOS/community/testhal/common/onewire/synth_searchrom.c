/*
    ChibiOS/RT - Copyright (C) 2014 Uladzimir Pylinsky aka barthess

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

#include <stdlib.h>

/*
 ******************************************************************************
 * DEFINES
 ******************************************************************************
 */

/* do not set it more than 64 because of some fill_pattern functions
   will be broken.*/
#define SYNTH_DEVICES_MAX     64

/*
 * synthetic device
 */
typedef struct {
  bool      active;
  uint64_t  id;
} OWSynthDevice;

/*
 * synthetic bus
 */
typedef struct {
  OWSynthDevice   devices[SYNTH_DEVICES_MAX];
  size_t          dev_present;
  bool            complement_bit;
  ioline_t        rom_bit;
} OWSynthBus;

/*
 ******************************************************************************
 * EXTERNS
 ******************************************************************************
 */

/*
 ******************************************************************************
 * PROTOTYPES
 ******************************************************************************
 */

/*
 ******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************
 */

static OWSynthBus synth_bus;

/*
 * local buffer for discovered ROMs
 */
static uint64_t detected_devices[SYNTH_DEVICES_MAX];

/*
 ******************************************************************************
 ******************************************************************************
 * LOCAL FUNCTIONS
 ******************************************************************************
 ******************************************************************************
 */

/*
 ******************************************************************************
 * EXPORTED FUNCTIONS
 ******************************************************************************
 */

/*
 *
 */
void _synth_ow_write_bit(onewireDriver *owp, ioline_t bit) {
  (void)owp;
  size_t i;

  for (i=0; i<SYNTH_DEVICES_MAX; i++) {
    if (((synth_bus.devices[i].id >> synth_bus.rom_bit) & 1U) != bit) {
      synth_bus.devices[i].active = false;
    }
  }
  synth_bus.rom_bit++;
}

/*
 *
 */
ioline_t _synth_ow_read_bit(void) {
  ioline_t ret = 0xFF;
  size_t i;
  ioline_t bit;

  for (i=0; i<SYNTH_DEVICES_MAX; i++) {
    if (synth_bus.devices[i].active){
      bit = (synth_bus.devices[i].id >> synth_bus.rom_bit) & 1U;
      if (synth_bus.complement_bit){
        bit ^= 1U;
      }
      if (0xFF == ret)
        ret = bit;
      else
        ret &= bit;
    }
  }
  synth_bus.complement_bit = !synth_bus.complement_bit;
  return ret;
}

/*
 *
 */
static void synth_reset_pulse(void){
  size_t i;

  for (i=0; i<synth_bus.dev_present; i++){
    synth_bus.devices[i].active = true;
  }
}

/*
 *
 */
static size_t synth_search_rom(onewireDriver *owp, uint8_t *result, size_t max_rom_cnt) {

  size_t i;

  search_clean_start(&owp->search_rom);

  do {
    /* initialize buffer to store result */
    if (owp->search_rom.reg.devices_found >= max_rom_cnt)
      owp->search_rom.retbuf = result + 8*(max_rom_cnt-1);
    else
      owp->search_rom.retbuf = result + 8*owp->search_rom.reg.devices_found;
    memset(owp->search_rom.retbuf, 0, 8);

    /* clean iteration state */
    search_clean_iteration(&owp->search_rom);

    /**/
    synth_reset_pulse();
    synth_bus.rom_bit = 0;
    synth_bus.complement_bit = false;
    for (i=0; i<64*3 - 1; i++){
      ow_search_rom_cb(NULL, owp);
    }

    if (ONEWIRE_SEARCH_ROM_ERROR != owp->search_rom.reg.result) {
      /* store cached result for usage in next iteration */
      memcpy(owp->search_rom.prev_path, owp->search_rom.retbuf, 8);
    }
  }
  while (ONEWIRE_SEARCH_ROM_SUCCESS == owp->search_rom.reg.result);

  /**/
  if (ONEWIRE_SEARCH_ROM_ERROR == owp->search_rom.reg.result)
    return 0;
  else
    return owp->search_rom.reg.devices_found;
}

/*
 *
 */
static void fill_pattern_real_devices(void) {
  size_t i;

  for (i=0; i<SYNTH_DEVICES_MAX; i++)
    synth_bus.devices[i].active = false;

  synth_bus.devices[0].active = true;
  synth_bus.devices[0].id = 0x1d00000567f5ec28;

  synth_bus.devices[1].active = true;
  synth_bus.devices[1].id = 0x37000005601abd28;

  synth_bus.devices[2].active = true;
  synth_bus.devices[2].id = 0x0f000005677d8328;
}

/*
 *
 */
static void fill_pattern_00(size_t devices, size_t start) {
  size_t i;

  for (i=0; i<SYNTH_DEVICES_MAX; i++)
    synth_bus.devices[i].active = false;

  for (i=0; i<devices; i++){
    synth_bus.devices[i].active = true;
    synth_bus.devices[i].id = (start + i);
  }
}

/*
 *
 */
static void fill_pattern_01(size_t devices) {
  size_t i;

  for (i=0; i<SYNTH_DEVICES_MAX; i++)
    synth_bus.devices[i].active = false;

  for (i=0; i<devices; i++){
    synth_bus.devices[i].active = true;
    synth_bus.devices[i].id = (devices - i);
  }
}

/*
 *
 */
static void fill_pattern_02(size_t devices) {
  size_t i;

  for (i=0; i<SYNTH_DEVICES_MAX; i++)
    synth_bus.devices[i].active = false;

  for (i=0; i<devices; i++){
    synth_bus.devices[i].active = true;
    synth_bus.devices[i].id = ((uint64_t)1 << i);
  }
}

/*
 *
 */
static void fill_pattern_03(size_t devices) {
  size_t i;

  for (i=0; i<SYNTH_DEVICES_MAX; i++)
    synth_bus.devices[i].active = false;

  for (i=0; i<devices; i++){
    synth_bus.devices[i].active = true;
    synth_bus.devices[i].id = ((uint64_t)0x8000000000000000 >> i);
  }
}

/*
 * Random pattern helper
 */
static bool is_id_uniq(const OWSynthDevice *dev, size_t n, uint64_t id) {
  size_t i;

  for (i=0; i<n; i++) {
    if (dev[i].id == id)
      return false;
  }
  return true;
}

/*
 *
 */
static void fill_pattern_rand(size_t devices) {
  size_t i;
  uint64_t new_id;

  for (i=0; i<SYNTH_DEVICES_MAX; i++){
    synth_bus.devices[i].active = false;
    synth_bus.devices[i].id = 0;
  }

  for (i=0; i<devices; i++) {
    do {
      new_id = rand();
      new_id = (new_id << 32) | rand();
    } while (true != is_id_uniq(synth_bus.devices, i, new_id));

    synth_bus.devices[i].id = new_id;
    synth_bus.devices[i].active = true;
  }
}

/*
 *
 */
static bool check_result(size_t detected) {

  size_t i,j;
  bool match = false;

  for (i=0; i<detected; i++){
    match = false;
    for (j=0; j<detected; j++){
      if (synth_bus.devices[i].id == detected_devices[j]){
        match = true;
        break;
      }
    }
    if (false == match)
      return OSAL_FAILED;
  }
  return OSAL_SUCCESS;
}

/*
 *
 */
void synthSearchRomTest(onewireDriver *owp) {

  size_t detected = 0;
  size_t i;

  synth_bus.dev_present = 3;
  fill_pattern_real_devices();
  detected = synth_search_rom(owp, (uint8_t *)detected_devices, SYNTH_DEVICES_MAX);
  osalDbgCheck(synth_bus.dev_present == detected);
  osalDbgCheck(OSAL_SUCCESS == check_result(detected));

  for (i=1; i<=SYNTH_DEVICES_MAX; i++){
    synth_bus.dev_present = i;

    fill_pattern_00(synth_bus.dev_present, 0);
    detected = synth_search_rom(owp, (uint8_t *)detected_devices, SYNTH_DEVICES_MAX);
    osalDbgCheck(synth_bus.dev_present == detected);
    osalDbgCheck(OSAL_SUCCESS == check_result(detected));

    fill_pattern_00(synth_bus.dev_present, 1);
    detected = synth_search_rom(owp, (uint8_t *)detected_devices, SYNTH_DEVICES_MAX);
    osalDbgCheck(synth_bus.dev_present == detected);
    osalDbgCheck(OSAL_SUCCESS == check_result(detected));

    fill_pattern_01(synth_bus.dev_present);
    detected = synth_search_rom(owp, (uint8_t *)detected_devices, SYNTH_DEVICES_MAX);
    osalDbgCheck(synth_bus.dev_present == detected);
    osalDbgCheck(OSAL_SUCCESS == check_result(detected));

    fill_pattern_02(synth_bus.dev_present);
    detected = synth_search_rom(owp, (uint8_t *)detected_devices, SYNTH_DEVICES_MAX);
    osalDbgCheck(synth_bus.dev_present == detected);
    osalDbgCheck(OSAL_SUCCESS == check_result(detected));

    fill_pattern_03(synth_bus.dev_present);
    detected = synth_search_rom(owp, (uint8_t *)detected_devices, SYNTH_DEVICES_MAX);
    osalDbgCheck(synth_bus.dev_present == detected);
    osalDbgCheck(OSAL_SUCCESS == check_result(detected));
  }

  i = 0;
  while (i < 1000) {
    synth_bus.dev_present = 1 + (rand() & 63);

    fill_pattern_rand(synth_bus.dev_present);
    detected = synth_search_rom(owp, (uint8_t *)detected_devices, SYNTH_DEVICES_MAX);
    osalDbgCheck(synth_bus.dev_present == detected);
    osalDbgCheck(OSAL_SUCCESS == check_result(detected));
    i++;
  }
}


