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

#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#include "platform.h"
#include "build/debug.h"
#include "build/version.h"

#include "common/maths.h"
#include "common/streambuf.h"
#include "common/utils.h"

#include "msp/msp.h"
#include "msp/msp_protocol.h"

#if defined(USE_ROBOT)

#include "fc/fc_robot.h"
#include "navigation/navigation_robot.h"

//static bool mspUnpackRobotCommand()

mspResult_e mspProcessRobotCommand(uint16_t cmdMSP, sbuf_t * dst, sbuf_t * src)
{
    navRobotMovement_t move;

    switch (cmdMSP) {
        case MSP2_INAV_ROBOT_GET_STATUS:
            return MSP_RESULT_ACK;

        case MSP2_INAV_ROBOT_CMD_STOP:
            move.posMoveMode    = NAV_ROBOT_MOVE_RELATIVE;
            move.headMoveMode   = NAV_ROBOT_MOVE_RELATIVE;
            move.posMotion.x    = 0;
            move.posMotion.y    = 0;
            move.posMotion.z    = 0;
            move.headMotion     = 0;
            navRobotModeMoveHandler(&move, false);
            return MSP_RESULT_ACK;

        case MSP2_INAV_ROBOT_CMD_MOVE:
            move.posMoveMode    = sbufReadU8(src);
            move.headMoveMode   = sbufReadU8(src);
            move.posMotion.x    = ((int32_t)sbufReadU32(src));
            move.posMotion.y    = ((int32_t)sbufReadU32(src));
            move.posMotion.z    = ((int32_t)sbufReadU32(src));
            move.headMotion     = ((int16_t)sbufReadU16(src));
            navRobotModeMoveHandler(&move, false);
            return MSP_RESULT_ACK;

        case MSP2_INAV_ROBOT_CMD_MOVE_BODY_FRAME:
            move.posMoveMode    = sbufReadU8(src);
            move.headMoveMode   = sbufReadU8(src);
            move.posMotion.x    = ((int32_t)sbufReadU32(src));
            move.posMotion.y    = ((int32_t)sbufReadU32(src));
            move.posMotion.z    = ((int32_t)sbufReadU32(src));
            move.headMotion     = ((int16_t)sbufReadU16(src));
            navRobotModeMoveHandler(&move, true);
            return MSP_RESULT_ACK;

        default:
            return MSP_RESULT_ERROR;
    }

    // Error out by default
    return MSP_RESULT_ERROR;
}


#endif
