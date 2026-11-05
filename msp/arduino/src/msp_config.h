#pragma once
// Client-side limits for this library. These are NOT wire constants: nothing in
// the MSP schema defines them, they size the buffers this library allocates
// when talking to a flight controller. Override before including the library.
//
// Wire constants come from generated/msp_consts.h, which is generated from the
// schema and must not be edited.

#include <stdint.h>

// Largest values this client will accept from a FC. A FC configured for more
// simply gets truncated here rather than overflowing a buffer.
#ifndef MAX_RC_CHANNELS
#  define MAX_RC_CHANNELS 18
#endif
#ifndef MAX_MODE_RANGES
#  define MAX_MODE_RANGES 40
#endif
#ifndef MAX_NAME_LENGTH
#  define MAX_NAME_LENGTH 16
#endif
// MSP_BOARD_INFO carries the target name in a variable-length tail after a
// fixed header. Its wire length field is one byte, so this is the largest tail
// a reply can carry.
#ifndef MAX_TARGET_NAME_LENGTH
#  define MAX_TARGET_NAME_LENGTH 255
#endif
#ifndef NAV_MAX_WAYPOINTS
#  define NAV_MAX_WAYPOINTS 120
#endif
/* CHECKBOX_ITEM_COUNT is not defined here: it is the terminator of boxId_e,
 * which generated/msp_wire_types.h vendors verbatim from INAV. Defining it
 * would both mangle that enum and let the two drift apart. */

// The schema marks these build-configuration dependent, so msp_consts.h
// refuses to guess. A client does not know the FC's build, so it picks the
// widest value it is prepared to handle.
#ifndef MAX_SUPPORTED_MOTORS
#  define MAX_SUPPORTED_MOTORS 12
#endif
#ifndef HARDWARE_TIMER_DEFINITION_COUNT
#  define HARDWARE_TIMER_DEFINITION_COUNT 16
#endif

// RC pulse range, used by the SI-unit helpers in inav_api.
#ifndef PWM_RANGE_MIN
#  define PWM_RANGE_MIN 1000
#endif
#ifndef PWM_RANGE_MAX
#  define PWM_RANGE_MAX 2000
#endif
#ifndef PWM_RANGE_MIDDLE
#  define PWM_RANGE_MIDDLE (PWM_RANGE_MIN + ((PWM_RANGE_MAX - PWM_RANGE_MIN) / 2))
#endif
