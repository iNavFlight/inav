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

mspResult_e mspProcessRobotCommand(uint16_t cmdMSP, sbuf_t *src) 
{
    UNUSED(src);

    switch (cmdMSP) {
        case MSP2_INAV_ROBOT_GET_STATUS:
            break;
    }

    return MSP_RESULT_NO_REPLY;
}


#endif
