/*
 * This file is part of ATBetaflight and INav.
 *
 * ATBetaflight and INav are free software. You can redistribute
 * this software and/or modify this software under the terms of the
 * GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * ATBetaflight and INav are distributed in the hope that they
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: jflyper (https://github.com/jflyper)
 */

#pragma once

#include "emfat.h"

extern emfat_t emfat;


#ifdef __cplusplus
extern "C" {
#endif


#include "usb_conf.h"
#include "usb_std.h"



uint8_t *get_inquiry(uint8_t lun);
usb_sts_type msc_disk_read(uint8_t lun, uint32_t addr, uint8_t *read_buf, uint32_t len);
usb_sts_type msc_disk_write(uint8_t lun, uint32_t addr, uint8_t *buf, uint32_t len);
usb_sts_type msc_disk_capacity(uint8_t lun, uint32_t *blk_nbr, uint32_t *blk_size);
