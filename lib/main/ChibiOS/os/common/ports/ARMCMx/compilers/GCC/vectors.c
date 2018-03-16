/*
    ChibiOS - Copyright (C) 2006..2015 Giovanni Di Sirio

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

/**
 * @file    ARMCMx/compilers/GCC/vectors.c
 * @brief   Interrupt vectors for Cortex-Mx devices.
 *
 * @defgroup ARMCMx_VECTORS Cortex-Mx Interrupt Vectors
 * @{
 */

#include <stdbool.h>
#include <stdint.h>

#include "vectors.h"

#if (CORTEX_NUM_VECTORS % 8) != 0
#error "the constant CORTEX_NUM_VECTORS must be a multiple of 8"
#endif

#if (CORTEX_NUM_VECTORS < 8) || (CORTEX_NUM_VECTORS > 240)
#error "the constant CORTEX_NUM_VECTORS must be between 8 and 240 inclusive"
#endif

/**
 * @brief   Unhandled exceptions handler.
 * @details Any undefined exception vector points to this function by default.
 *          This function simply stops the system into an infinite loop.
 *
 * @notapi
 */
/*lint -save -e9075 [8.4] All symbols are invoked from asm context.*/
void _unhandled_exception(void) {
/*lint -restore*/

  while (true) {
  }
}

#if !defined(__DOXYGEN__)
extern uint32_t __main_stack_end__;
void Reset_Handler(void);
void NMI_Handler(void) __attribute__((weak, alias("_unhandled_exception")));
void HardFault_Handler(void) __attribute__((weak, alias("_unhandled_exception")));
void MemManage_Handler(void) __attribute__((weak, alias("_unhandled_exception")));
void BusFault_Handler(void) __attribute__((weak, alias("_unhandled_exception")));
void UsageFault_Handler(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector1C(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector20(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector24(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector28(void) __attribute__((weak, alias("_unhandled_exception")));
void SVC_Handler(void) __attribute__((weak, alias("_unhandled_exception")));
void DebugMon_Handler(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector34(void) __attribute__((weak, alias("_unhandled_exception")));
void PendSV_Handler(void) __attribute__((weak, alias("_unhandled_exception")));
void SysTick_Handler(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector40(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector44(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector48(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector4C(void) __attribute__((weak, alias("_unhandled_exception")));
#if CORTEX_NUM_VECTORS > 4
void Vector50(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector54(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector58(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector5C(void) __attribute__((weak, alias("_unhandled_exception")));
#endif
#if CORTEX_NUM_VECTORS > 8
void Vector60(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector64(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector68(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector6C(void) __attribute__((weak, alias("_unhandled_exception")));
#endif
#if CORTEX_NUM_VECTORS > 12
void Vector70(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector74(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector78(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector7C(void) __attribute__((weak, alias("_unhandled_exception")));
#endif
#if CORTEX_NUM_VECTORS > 16
void Vector80(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector84(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector88(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector8C(void) __attribute__((weak, alias("_unhandled_exception")));
#endif
#if CORTEX_NUM_VECTORS > 20
void Vector90(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector94(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector98(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector9C(void) __attribute__((weak, alias("_unhandled_exception")));
#endif
#if CORTEX_NUM_VECTORS > 24
void VectorA0(void) __attribute__((weak, alias("_unhandled_exception")));
void VectorA4(void) __attribute__((weak, alias("_unhandled_exception")));
void VectorA8(void) __attribute__((weak, alias("_unhandled_exception")));
void VectorAC(void) __attribute__((weak, alias("_unhandled_exception")));
#endif
#if CORTEX_NUM_VECTORS > 28
void VectorB0(void) __attribute__((weak, alias("_unhandled_exception")));
void VectorB4(void) __attribute__((weak, alias("_unhandled_exception")));
void VectorB8(void) __attribute__((weak, alias("_unhandled_exception")));
void VectorBC(void) __attribute__((weak, alias("_unhandled_exception")));
#endif
#if CORTEX_NUM_VECTORS > 32
void VectorC0(void) __attribute__((weak, alias("_unhandled_exception")));
void VectorC4(void) __attribute__((weak, alias("_unhandled_exception")));
void VectorC8(void) __attribute__((weak, alias("_unhandled_exception")));
void VectorCC(void) __attribute__((weak, alias("_unhandled_exception")));
#endif
#if CORTEX_NUM_VECTORS > 36
void VectorD0(void) __attribute__((weak, alias("_unhandled_exception")));
void VectorD4(void) __attribute__((weak, alias("_unhandled_exception")));
void VectorD8(void) __attribute__((weak, alias("_unhandled_exception")));
void VectorDC(void) __attribute__((weak, alias("_unhandled_exception")));
#endif
#if CORTEX_NUM_VECTORS > 40
void VectorE0(void) __attribute__((weak, alias("_unhandled_exception")));
void VectorE4(void) __attribute__((weak, alias("_unhandled_exception")));
void VectorE8(void) __attribute__((weak, alias("_unhandled_exception")));
void VectorEC(void) __attribute__((weak, alias("_unhandled_exception")));
#endif
#if CORTEX_NUM_VECTORS > 44
void VectorF0(void) __attribute__((weak, alias("_unhandled_exception")));
void VectorF4(void) __attribute__((weak, alias("_unhandled_exception")));
void VectorF8(void) __attribute__((weak, alias("_unhandled_exception")));
void VectorFC(void) __attribute__((weak, alias("_unhandled_exception")));
#endif
#if CORTEX_NUM_VECTORS > 48
void Vector100(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector104(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector108(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector10C(void) __attribute__((weak, alias("_unhandled_exception")));
#endif
#if CORTEX_NUM_VECTORS > 52
void Vector110(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector114(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector118(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector11C(void) __attribute__((weak, alias("_unhandled_exception")));
#endif
#if CORTEX_NUM_VECTORS > 56
void Vector120(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector124(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector128(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector12C(void) __attribute__((weak, alias("_unhandled_exception")));
#endif
#if CORTEX_NUM_VECTORS > 60
void Vector130(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector134(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector138(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector13C(void) __attribute__((weak, alias("_unhandled_exception")));
#endif
#if CORTEX_NUM_VECTORS > 64
void Vector140(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector144(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector148(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector14C(void) __attribute__((weak, alias("_unhandled_exception")));
#endif
#if CORTEX_NUM_VECTORS > 68
void Vector150(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector154(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector158(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector15C(void) __attribute__((weak, alias("_unhandled_exception")));
#endif
#if CORTEX_NUM_VECTORS > 72
void Vector160(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector164(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector168(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector16C(void) __attribute__((weak, alias("_unhandled_exception")));
#endif
#if CORTEX_NUM_VECTORS > 76
void Vector170(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector174(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector178(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector17C(void) __attribute__((weak, alias("_unhandled_exception")));
#endif
#if CORTEX_NUM_VECTORS > 80
void Vector180(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector184(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector188(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector18C(void) __attribute__((weak, alias("_unhandled_exception")));
#endif
#if CORTEX_NUM_VECTORS > 84
void Vector190(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector194(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector198(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector19C(void) __attribute__((weak, alias("_unhandled_exception")));
#endif
#if CORTEX_NUM_VECTORS > 88
void Vector1A0(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector1A4(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector1A8(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector1AC(void) __attribute__((weak, alias("_unhandled_exception")));
#endif
#if CORTEX_NUM_VECTORS > 92
void Vector1B0(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector1B4(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector1B8(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector1BC(void) __attribute__((weak, alias("_unhandled_exception")));
#endif
#if CORTEX_NUM_VECTORS > 96
void Vector1C0(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector1C4(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector1C8(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector1CC(void) __attribute__((weak, alias("_unhandled_exception")));
#endif
#if CORTEX_NUM_VECTORS > 100
void Vector1D0(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector1D4(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector1D8(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector1DC(void) __attribute__((weak, alias("_unhandled_exception")));
#endif
#if CORTEX_NUM_VECTORS > 104
void Vector1E0(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector1E4(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector1E8(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector1EC(void) __attribute__((weak, alias("_unhandled_exception")));
#endif
#if CORTEX_NUM_VECTORS > 108
void Vector1F0(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector1F4(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector1F8(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector1FC(void) __attribute__((weak, alias("_unhandled_exception")));
#endif
#if CORTEX_NUM_VECTORS > 112
void Vector200(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector204(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector208(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector20C(void) __attribute__((weak, alias("_unhandled_exception")));
#endif
#if CORTEX_NUM_VECTORS > 116
void Vector210(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector214(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector218(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector21C(void) __attribute__((weak, alias("_unhandled_exception")));
#endif
#if CORTEX_NUM_VECTORS > 120
void Vector220(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector224(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector228(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector22C(void) __attribute__((weak, alias("_unhandled_exception")));
#endif
#if CORTEX_NUM_VECTORS > 124
void Vector230(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector234(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector238(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector23C(void) __attribute__((weak, alias("_unhandled_exception")));
#endif
#if CORTEX_NUM_VECTORS > 128
void Vector240(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector244(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector248(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector24C(void) __attribute__((weak, alias("_unhandled_exception")));
#endif
#if CORTEX_NUM_VECTORS > 132
void Vector250(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector254(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector258(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector25C(void) __attribute__((weak, alias("_unhandled_exception")));
#endif
#if CORTEX_NUM_VECTORS > 136
void Vector260(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector264(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector268(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector26C(void) __attribute__((weak, alias("_unhandled_exception")));
#endif
#if CORTEX_NUM_VECTORS > 140
void Vector270(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector274(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector278(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector27C(void) __attribute__((weak, alias("_unhandled_exception")));
#endif
#if CORTEX_NUM_VECTORS > 144
void Vector280(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector284(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector288(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector28C(void) __attribute__((weak, alias("_unhandled_exception")));
#endif
#if CORTEX_NUM_VECTORS > 148
void Vector290(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector294(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector298(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector29C(void) __attribute__((weak, alias("_unhandled_exception")));
#endif
#if CORTEX_NUM_VECTORS > 152
void Vector2A0(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector2A4(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector2A8(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector2AC(void) __attribute__((weak, alias("_unhandled_exception")));
#endif
#if CORTEX_NUM_VECTORS > 156
void Vector2B0(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector2B4(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector2B8(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector2BC(void) __attribute__((weak, alias("_unhandled_exception")));
#endif
#if CORTEX_NUM_VECTORS > 160
void Vector2C0(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector2C4(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector2C8(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector2CC(void) __attribute__((weak, alias("_unhandled_exception")));
#endif
#if CORTEX_NUM_VECTORS > 164
void Vector2D0(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector2D4(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector2D8(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector2DC(void) __attribute__((weak, alias("_unhandled_exception")));
#endif
#if CORTEX_NUM_VECTORS > 168
void Vector2E0(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector2E4(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector2E8(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector2EC(void) __attribute__((weak, alias("_unhandled_exception")));
#endif
#if CORTEX_NUM_VECTORS > 172
void Vector2F0(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector2F4(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector2F8(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector2FC(void) __attribute__((weak, alias("_unhandled_exception")));
#endif
#if CORTEX_NUM_VECTORS > 176
void Vector300(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector304(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector308(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector30C(void) __attribute__((weak, alias("_unhandled_exception")));
#endif
#if CORTEX_NUM_VECTORS > 180
void Vector310(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector314(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector318(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector31C(void) __attribute__((weak, alias("_unhandled_exception")));
#endif
#if CORTEX_NUM_VECTORS > 184
void Vector320(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector324(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector328(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector32C(void) __attribute__((weak, alias("_unhandled_exception")));
#endif
#if CORTEX_NUM_VECTORS > 188
void Vector330(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector334(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector338(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector33C(void) __attribute__((weak, alias("_unhandled_exception")));
#endif
#if CORTEX_NUM_VECTORS > 192
void Vector340(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector344(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector348(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector34C(void) __attribute__((weak, alias("_unhandled_exception")));
#endif
#if CORTEX_NUM_VECTORS > 196
void Vector350(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector354(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector358(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector35C(void) __attribute__((weak, alias("_unhandled_exception")));
#endif
#if CORTEX_NUM_VECTORS > 200
void Vector360(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector364(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector368(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector36C(void) __attribute__((weak, alias("_unhandled_exception")));
#endif
#if CORTEX_NUM_VECTORS > 204
void Vector370(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector374(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector378(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector37C(void) __attribute__((weak, alias("_unhandled_exception")));
#endif
#if CORTEX_NUM_VECTORS > 208
void Vector380(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector384(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector388(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector38C(void) __attribute__((weak, alias("_unhandled_exception")));
#endif
#if CORTEX_NUM_VECTORS > 212
void Vector390(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector394(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector398(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector39C(void) __attribute__((weak, alias("_unhandled_exception")));
#endif
#if CORTEX_NUM_VECTORS > 216
void Vector3A0(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector3A4(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector3A8(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector3AC(void) __attribute__((weak, alias("_unhandled_exception")));
#endif
#if CORTEX_NUM_VECTORS > 220
void Vector3B0(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector3B4(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector3B8(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector3BC(void) __attribute__((weak, alias("_unhandled_exception")));
#endif
#if CORTEX_NUM_VECTORS > 224
void Vector3C0(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector3C4(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector3C8(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector3CC(void) __attribute__((weak, alias("_unhandled_exception")));
#endif
#if CORTEX_NUM_VECTORS > 228
void Vector3D0(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector3D4(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector3D8(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector3DC(void) __attribute__((weak, alias("_unhandled_exception")));
#endif
#if CORTEX_NUM_VECTORS > 232
void Vector3E0(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector3E4(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector3E8(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector3EC(void) __attribute__((weak, alias("_unhandled_exception")));
#endif
#if CORTEX_NUM_VECTORS > 236
void Vector3F0(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector3F4(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector3F8(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector3FC(void) __attribute__((weak, alias("_unhandled_exception")));
#endif
#endif /* !defined(__DOXYGEN__) */

/**
 * @brief   STM32 vectors table.
 */
#if !defined(__DOXYGEN__)
#if !defined(VECTORS_SECTION)
__attribute__ ((used, aligned(128), section(".vectors")))
#else
__attribute__ ((used, aligned(128), section(VECTORS_SECTION)))
#endif
#endif
/*lint -save -e9075 [8.4] All symbols are invoked from asm context.*/
vectors_t _vectors = {
/*lint -restore*/
  &__main_stack_end__,Reset_Handler,      NMI_Handler,        HardFault_Handler,
  MemManage_Handler,  BusFault_Handler,   UsageFault_Handler, Vector1C,
  Vector20,           Vector24,           Vector28,           SVC_Handler,
  DebugMon_Handler,   Vector34,           PendSV_Handler,     SysTick_Handler,
  {
    Vector40,           Vector44,           Vector48,           Vector4C,
#if CORTEX_NUM_VECTORS > 4
    Vector50,           Vector54,           Vector58,           Vector5C,
#endif
#if CORTEX_NUM_VECTORS > 8
    Vector60,           Vector64,           Vector68,           Vector6C,
#endif
#if CORTEX_NUM_VECTORS > 12
    Vector70,           Vector74,           Vector78,           Vector7C,
#endif
#if CORTEX_NUM_VECTORS > 16
    Vector80,           Vector84,           Vector88,           Vector8C,
#endif
#if CORTEX_NUM_VECTORS > 20
    Vector90,           Vector94,           Vector98,           Vector9C,
#endif
#if CORTEX_NUM_VECTORS > 24
    VectorA0,           VectorA4,           VectorA8,           VectorAC,
#endif
#if CORTEX_NUM_VECTORS > 28
    VectorB0,           VectorB4,           VectorB8,           VectorBC,
#endif
#if CORTEX_NUM_VECTORS > 32
    VectorC0,           VectorC4,           VectorC8,           VectorCC,
#endif
#if CORTEX_NUM_VECTORS > 36
    VectorD0,           VectorD4,           VectorD8,           VectorDC,
#endif
#if CORTEX_NUM_VECTORS > 40
    VectorE0,           VectorE4,           VectorE8,           VectorEC,
#endif
#if CORTEX_NUM_VECTORS > 44
    VectorF0,           VectorF4,           VectorF8,           VectorFC,
#endif
#if CORTEX_NUM_VECTORS > 48
    Vector100,          Vector104,          Vector108,          Vector10C,
#endif
#if CORTEX_NUM_VECTORS > 52
    Vector110,          Vector114,          Vector118,          Vector11C,
#endif
#if CORTEX_NUM_VECTORS > 56
    Vector120,          Vector124,          Vector128,          Vector12C,
#endif
#if CORTEX_NUM_VECTORS > 60
    Vector130,          Vector134,          Vector138,          Vector13C,
#endif
#if CORTEX_NUM_VECTORS > 64
    Vector140,          Vector144,          Vector148,          Vector14C,
#endif
#if CORTEX_NUM_VECTORS > 68
    Vector150,          Vector154,          Vector158,          Vector15C,
#endif
#if CORTEX_NUM_VECTORS > 72
    Vector160,          Vector164,          Vector168,          Vector16C,
#endif
#if CORTEX_NUM_VECTORS > 76
    Vector170,          Vector174,          Vector178,          Vector17C,
#endif
#if CORTEX_NUM_VECTORS > 80
    Vector180,          Vector184,          Vector188,          Vector18C,
#endif
#if CORTEX_NUM_VECTORS > 84
    Vector190,          Vector194,          Vector198,          Vector19C,
#endif
#if CORTEX_NUM_VECTORS > 88
    Vector1A0,          Vector1A4,          Vector1A8,          Vector1AC,
#endif
#if CORTEX_NUM_VECTORS > 92
    Vector1B0,          Vector1B4,          Vector1B8,          Vector1BC,
#endif
#if CORTEX_NUM_VECTORS > 96
    Vector1C0,          Vector1C4,          Vector1C8,          Vector1CC,
#endif
#if CORTEX_NUM_VECTORS > 100
    Vector1D0,          Vector1D4,          Vector1D8,          Vector1DC,
#endif
#if CORTEX_NUM_VECTORS > 104
    Vector1E0,          Vector1E4,          Vector1E8,          Vector1EC,
#endif
#if CORTEX_NUM_VECTORS > 108
    Vector1F0,          Vector1F4,          Vector1F8,          Vector1FC,
#endif
#if CORTEX_NUM_VECTORS > 112
    Vector200,          Vector204,          Vector208,          Vector20C,
#endif
#if CORTEX_NUM_VECTORS > 116
    Vector210,          Vector214,          Vector218,          Vector21C,
#endif
#if CORTEX_NUM_VECTORS > 120
    Vector220,          Vector224,          Vector228,          Vector22C,
#endif
#if CORTEX_NUM_VECTORS > 124
    Vector230,          Vector234,          Vector238,          Vector23C,
#endif
#if CORTEX_NUM_VECTORS > 128
    Vector240,          Vector244,          Vector248,          Vector24C,
#endif
#if CORTEX_NUM_VECTORS > 132
    Vector250,          Vector254,          Vector258,          Vector25C,
#endif
#if CORTEX_NUM_VECTORS > 136
    Vector260,          Vector264,          Vector268,          Vector26C,
#endif
#if CORTEX_NUM_VECTORS > 140
    Vector270,          Vector274,          Vector278,          Vector27C,
#endif
#if CORTEX_NUM_VECTORS > 144
    Vector280,          Vector284,          Vector288,          Vector28C,
#endif
#if CORTEX_NUM_VECTORS > 148
    Vector290,          Vector294,          Vector298,          Vector29C,
#endif
#if CORTEX_NUM_VECTORS > 152
    Vector2A0,          Vector2A4,          Vector2A8,          Vector2AC,
#endif
#if CORTEX_NUM_VECTORS > 156
    Vector2B0,          Vector2B4,          Vector2B8,          Vector2BC,
#endif
#if CORTEX_NUM_VECTORS > 160
    Vector2C0,          Vector2C4,          Vector2C8,          Vector2CC,
#endif
#if CORTEX_NUM_VECTORS > 164
    Vector2D0,          Vector2D4,          Vector2D8,          Vector2DC,
#endif
#if CORTEX_NUM_VECTORS > 168
    Vector2E0,          Vector2E4,          Vector2E8,          Vector2EC,
#endif
#if CORTEX_NUM_VECTORS > 172
    Vector2F0,          Vector2F4,          Vector2F8,          Vector2FC,
#endif
#if CORTEX_NUM_VECTORS > 176
    Vector300,          Vector304,          Vector308,          Vector30C,
#endif
#if CORTEX_NUM_VECTORS > 180
    Vector310,          Vector314,          Vector318,          Vector31C,
#endif
#if CORTEX_NUM_VECTORS > 184
    Vector320,          Vector324,          Vector328,          Vector32C,
#endif
#if CORTEX_NUM_VECTORS > 188
    Vector330,          Vector334,          Vector338,          Vector33C,
#endif
#if CORTEX_NUM_VECTORS > 192
    Vector340,          Vector344,          Vector348,          Vector34C,
#endif
#if CORTEX_NUM_VECTORS > 196
    Vector350,          Vector354,          Vector358,          Vector35C,
#endif
#if CORTEX_NUM_VECTORS > 200
    Vector360,          Vector364,          Vector368,          Vector36C,
#endif
#if CORTEX_NUM_VECTORS > 204
    Vector370,          Vector374,          Vector378,          Vector37C,
#endif
#if CORTEX_NUM_VECTORS > 208
    Vector380,          Vector384,          Vector388,          Vector38C,
#endif
#if CORTEX_NUM_VECTORS > 212
    Vector390,          Vector394,          Vector398,          Vector39C,
#endif
#if CORTEX_NUM_VECTORS > 216
    Vector3A0,          Vector3A4,          Vector3A8,          Vector3AC,
#endif
#if CORTEX_NUM_VECTORS > 220
    Vector3B0,          Vector3B4,          Vector3B8,          Vector3BC,
#endif
#if CORTEX_NUM_VECTORS > 224
    Vector3C0,          Vector3C4,          Vector3C8,          Vector3CC,
#endif
#if CORTEX_NUM_VECTORS > 228
    Vector3D0,          Vector3D4,          Vector3D8,          Vector3DC
#endif
#if CORTEX_NUM_VECTORS > 232
    Vector3E0,          Vector3E4,          Vector3E8,          Vector3EC
#endif
#if CORTEX_NUM_VECTORS > 236
    Vector3F0,          Vector3F4,          Vector3F8,          Vector3FC
#endif
  }
};

/** @} */
