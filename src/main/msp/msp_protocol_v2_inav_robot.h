/*/*
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

#define MSP2_INAV_IS_ROBOT_MESSAGE(x)               ((x) >= 0x2F00 && (x) <= 0x2FFF)


#define MSP2_INAV_ROBOT_GET_STATUS                  0x2F00
/* Gets navigation system state. Takes no arguments.
 */

#define MSP2_INAV_ROBOT_CMD_STOP                    0x2F01
/* Aborts current motion. Takes no arguments.
 */


#define MSP2_INAV_ROBOT_CMD_MOVE                    0x2F02
/* Moves the current PosHold position in absolute frame of reference
 *  uint8_t  : position move mode (0: noop, 1: absolute, 2: relative, 3: incremental)
 *  uint8_t  : heading move mode (0: noop, 1: absolute, 2: relative, 3: incremental)
 *  uint32_t : position X (North), centimeters
 *  uint32_t : position Y (East), centimeters
 *  uint32_t : position Z (Up), centimeters
 *  uint16_t : heading, degrees, clockwise
 */

#define MSP2_INAV_ROBOT_CMD_MOVE_BODY_FRAME         0x2F03
/* Moves the current PosHold position in relative (drone) frame of reference
 *  uint8_t  : position move mode (0: noop, 1: absolute, 2: relative, 3: incremental)
 *  uint8_t  : heading move mode (0: noop, 1: absolute, 2: relative, 3: incremental)
 *  uint32_t : position X (Forward), centimeters
 *  uint32_t : position Y (Right), centimeters
 *  uint32_t : position Z (Up), centimeters
 *  uint16_t : heading, degrees, clockwise
 */
