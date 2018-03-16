[#ftl]
[@pp.dropOutputFile /]
[@pp.changeOutputFile name="mcuconf.h" /]
/*
    SPC5 HAL - Copyright (C) 2013 STMicroelectronics

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

#ifndef _MCUCONF_H_
#define _MCUCONF_H_

/*
 * SPC57EMxx drivers configuration.
 * The following settings override the default settings present in
 * the various device driver implementation headers.
 * Note that the settings for each driver only have effect if the whole
 * driver is enabled in halconf.h.
 *
 * IRQ priorities:
 * 1...15       Lowest...Highest.
 * DMA priorities:
 * 0...15       Highest...Lowest.
 */

#define SPC57EMxx_MCUCONF

/*
 * HAL driver system settings.
 */
#define SPC5_NO_INIT                        ${conf.instance.initialization_settings.do_not_init.value[0]?upper_case}
#define SPC5_ALLOW_OVERCLOCK                ${conf.instance.initialization_settings.allow_overclocking.value[0]?upper_case}
#define SPC5_DISABLE_WATCHDOG               ${conf.instance.initialization_settings.disable_watchdog.value[0]?upper_case}
#define SPC5_HSM_HANDSHAKE                  ${conf.instance.initialization_settings.hsm_handshake.@index[0]?trim}
#define SPC5_CLOCK_FAILURE_HOOK()           ${conf.instance.initialization_settings.clock_failure_hook.value[0]}
#define SPC5_PLL0_PREDIV_VALUE              ${conf.instance.initialization_settings.pll0_settings.prediv_value.value[0]}
#define SPC5_PLL0_MFD_VALUE                 ${conf.instance.initialization_settings.pll0_settings.mfd_value.value[0]}
#define SPC5_PLL0_RFDPHI_VALUE              ${conf.instance.initialization_settings.pll0_settings.rfdphi_value.value[0]}
#define SPC5_PLL0_RFDPHI1_VALUE             ${conf.instance.initialization_settings.pll0_settings.rfdphi1_value.value[0]}
#define SPC5_PLL1_MFD_VALUE                 ${conf.instance.initialization_settings.pll1_settings.mfd_value.value[0]}
#define SPC5_PLL1_RFDPHI_VALUE              ${conf.instance.initialization_settings.pll1_settings.rfdphi_value.value[0]}
#define SPC5_CGM_SC_DC0_DIV_VALUE           ${conf.instance.initialization_settings.cgm_settings.sc_dc0_divider.value[0]}
#define SPC5_CGM_SC_DC1_DIV_VALUE           ${conf.instance.initialization_settings.cgm_settings.sc_dc1_divider.value[0]}
#define SPC5_CGM_SC_DC2_DIV_VALUE           ${conf.instance.initialization_settings.cgm_settings.sc_dc2_divider.value[0]}
#define SPC5_CGM_AC0_SC_BITS                SPC5_CGM_SC_${conf.instance.initialization_settings.cgm_settings.ac0_clock_source.value[0]}
#define SPC5_CGM_AC0_DC0_DIV_VALUE          ${conf.instance.initialization_settings.cgm_settings.ac0_dc0_divider.value[0]}
#define SPC5_CGM_AC0_DC1_DIV_VALUE          ${conf.instance.initialization_settings.cgm_settings.ac0_dc1_divider.value[0]}
#define SPC5_CGM_AC0_DC2_DIV_VALUE          ${conf.instance.initialization_settings.cgm_settings.ac0_dc2_divider.value[0]}
#define SPC5_CGM_AC0_DC3_DIV_VALUE          ${conf.instance.initialization_settings.cgm_settings.ac0_dc3_divider.value[0]}
#define SPC5_CGM_AC0_DC3_DIV_FMT_VALUE      ${conf.instance.initialization_settings.cgm_settings.ac0_dc3_fmt.value[0]}
#define SPC5_CGM_AC0_DC4_DIV_VALUE          ${conf.instance.initialization_settings.cgm_settings.ac0_dc4_divider.value[0]}
#define SPC5_CGM_AC3_SC_BITS                SPC5_CGM_SC_${conf.instance.initialization_settings.cgm_settings.ac3_clock_source.value[0]}
#define SPC5_CGM_AC4_SC_BITS                SPC5_CGM_SC_${conf.instance.initialization_settings.cgm_settings.ac4_clock_source.value[0]}
#define SPC5_CGM_AC6_SC_BITS                SPC5_CGM_SC_${conf.instance.initialization_settings.cgm_settings.ac6_clock_source.value[0]}
#define SPC5_CGM_AC6_DC0_DIV_VALUE          ${conf.instance.initialization_settings.cgm_settings.ac6_dc0_divider.value[0]}
#define SPC5_CGM_AC7_SC_BITS                SPC5_CGM_SC_${conf.instance.initialization_settings.cgm_settings.ac7_clock_source.value[0]}
#define SPC5_CGM_AC7_DC0_DIV_VALUE          ${conf.instance.initialization_settings.cgm_settings.ac7_dc0_divider.value[0]}
#define SPC5_ME_ME_BITS                     (SPC5_ME_ME_RUN1 |              \
                                             SPC5_ME_ME_RUN2 |              \
                                             SPC5_ME_ME_RUN3 |              \
                                             SPC5_ME_ME_HALT0 |             \
                                             SPC5_ME_ME_STOP0)
#define SPC5_ME_SAFE_MC_BITS                (SPC5_ME_MC_PDO)
#define SPC5_ME_DRUN_MC_BITS                (SPC5_ME_MC_SYSCLK_PLL1PHI |    \
                                             SPC5_ME_MC_IRCON |             \
                                             SPC5_ME_MC_XOSC0ON |           \
                                             SPC5_ME_MC_PLL0ON |            \
                                             SPC5_ME_MC_PLL1ON |            \
                                             SPC5_ME_MC_FLAON_NORMAL |      \
                                             SPC5_ME_MC_MVRON)
#define SPC5_ME_RUN0_MC_BITS                (SPC5_ME_MC_SYSCLK_PLL1PHI |    \
                                             SPC5_ME_MC_IRCON |             \
                                             SPC5_ME_MC_XOSC0ON |           \
                                             SPC5_ME_MC_PLL0ON |            \
                                             SPC5_ME_MC_PLL1ON |            \
                                             SPC5_ME_MC_FLAON_NORMAL |      \
                                             SPC5_ME_MC_MVRON)
#define SPC5_ME_RUN1_MC_BITS                (SPC5_ME_MC_SYSCLK_PLL1PHI |    \
                                             SPC5_ME_MC_IRCON |             \
                                             SPC5_ME_MC_XOSC0ON |           \
                                             SPC5_ME_MC_PLL0ON |            \
                                             SPC5_ME_MC_PLL1ON |            \
                                             SPC5_ME_MC_FLAON_NORMAL |      \
                                             SPC5_ME_MC_MVRON)
#define SPC5_ME_RUN2_MC_BITS                (SPC5_ME_MC_SYSCLK_PLL1PHI |    \
                                             SPC5_ME_MC_IRCON |             \
                                             SPC5_ME_MC_XOSC0ON |           \
                                             SPC5_ME_MC_PLL0ON |            \
                                             SPC5_ME_MC_PLL1ON |            \
                                             SPC5_ME_MC_FLAON_NORMAL |      \
                                             SPC5_ME_MC_MVRON)
#define SPC5_ME_RUN3_MC_BITS                (SPC5_ME_MC_SYSCLK_PLL1PHI |    \
                                             SPC5_ME_MC_IRCON |             \
                                             SPC5_ME_MC_XOSC0ON |           \
                                             SPC5_ME_MC_PLL0ON |            \
                                             SPC5_ME_MC_PLL1ON |            \
                                             SPC5_ME_MC_FLAON_NORMAL |      \
                                             SPC5_ME_MC_MVRON)
#define SPC5_ME_HALT0_MC_BITS               (SPC5_ME_MC_SYSCLK_PLL1PHI |    \
                                             SPC5_ME_MC_IRCON |             \
                                             SPC5_ME_MC_XOSC0ON |           \
                                             SPC5_ME_MC_PLL0ON |            \
                                             SPC5_ME_MC_PLL1ON |            \
                                             SPC5_ME_MC_FLAON_NORMAL |      \
                                             SPC5_ME_MC_MVRON)
#define SPC5_ME_STOP0_MC_BITS               (SPC5_ME_MC_SYSCLK_PLL1PHI |    \
                                             SPC5_ME_MC_IRCON |             \
                                             SPC5_ME_MC_XOSC0ON |           \
                                             SPC5_ME_MC_PLL0ON |            \
                                             SPC5_ME_MC_PLL1ON |            \
                                             SPC5_ME_MC_FLAON_NORMAL |      \
                                             SPC5_ME_MC_MVRON)
#define SPC5_ME_RUN_PC0_BITS                0
#define SPC5_ME_RUN_PC1_BITS                (SPC5_ME_RUN_PC_SAFE |          \
                                             SPC5_ME_RUN_PC_DRUN |          \
                                             SPC5_ME_RUN_PC_RUN0 |          \
                                             SPC5_ME_RUN_PC_RUN1 |          \
                                             SPC5_ME_RUN_PC_RUN2 |          \
                                             SPC5_ME_RUN_PC_RUN3)
#define SPC5_ME_RUN_PC2_BITS                (SPC5_ME_RUN_PC_DRUN |          \
                                             SPC5_ME_RUN_PC_RUN0 |          \
                                             SPC5_ME_RUN_PC_RUN1 |          \
                                             SPC5_ME_RUN_PC_RUN2 |          \
                                             SPC5_ME_RUN_PC_RUN3)
#define SPC5_ME_RUN_PC3_BITS                (SPC5_ME_RUN_PC_DRUN |          \
                                             SPC5_ME_RUN_PC_RUN0 |          \
                                             SPC5_ME_RUN_PC_RUN1 |          \
                                             SPC5_ME_RUN_PC_RUN2 |          \
                                             SPC5_ME_RUN_PC_RUN3)
#define SPC5_ME_RUN_PC4_BITS                (SPC5_ME_RUN_PC_DRUN |          \
                                             SPC5_ME_RUN_PC_RUN0 |          \
                                             SPC5_ME_RUN_PC_RUN1 |          \
                                             SPC5_ME_RUN_PC_RUN2 |          \
                                             SPC5_ME_RUN_PC_RUN3)
#define SPC5_ME_RUN_PC5_BITS                (SPC5_ME_RUN_PC_DRUN |          \
                                             SPC5_ME_RUN_PC_RUN0 |          \
                                             SPC5_ME_RUN_PC_RUN1 |          \
                                             SPC5_ME_RUN_PC_RUN2 |          \
                                             SPC5_ME_RUN_PC_RUN3)
#define SPC5_ME_RUN_PC6_BITS                (SPC5_ME_RUN_PC_DRUN |          \
                                             SPC5_ME_RUN_PC_RUN0 |          \
                                             SPC5_ME_RUN_PC_RUN1 |          \
                                             SPC5_ME_RUN_PC_RUN2 |          \
                                             SPC5_ME_RUN_PC_RUN3)
#define SPC5_ME_RUN_PC7_BITS                (SPC5_ME_RUN_PC_DRUN |          \
                                             SPC5_ME_RUN_PC_RUN0 |          \
                                             SPC5_ME_RUN_PC_RUN1 |          \
                                             SPC5_ME_RUN_PC_RUN2 |          \
                                             SPC5_ME_RUN_PC_RUN3)
#define SPC5_ME_LP_PC0_BITS                 0
#define SPC5_ME_LP_PC1_BITS                 (SPC5_ME_LP_PC_HALT0 |          \
                                             SPC5_ME_LP_PC_STOP0)
#define SPC5_ME_LP_PC2_BITS                 (SPC5_ME_LP_PC_HALT0)
#define SPC5_ME_LP_PC3_BITS                 (SPC5_ME_LP_PC_STOP0)
#define SPC5_ME_LP_PC4_BITS                 (SPC5_ME_LP_PC_HALT0 |          \
                                             SPC5_ME_LP_PC_STOP0)
#define SPC5_ME_LP_PC5_BITS                 (SPC5_ME_LP_PC_HALT0 |          \
                                             SPC5_ME_LP_PC_STOP0)
#define SPC5_ME_LP_PC6_BITS                 (SPC5_ME_LP_PC_HALT0 |          \
                                             SPC5_ME_LP_PC_STOP0)
#define SPC5_ME_LP_PC7_BITS                 (SPC5_ME_LP_PC_HALT0 |          \
                                             SPC5_ME_LP_PC_STOP0)
#define SPC5_SSCM_ERROR_INIT                (SPC5_SSCM_ERROR_PAE |          \
                                             SPC5_SSCM_ERROR_RAE)

/*
 * SERIAL driver system settings.
 */
#define SPC5_SERIAL_USE_LINFLEX0            ${(conf.instance.linflex_settings.linflex0.value[0] == "Serial")?string?upper_case}
#define SPC5_SERIAL_USE_LINFLEX1            ${(conf.instance.linflex_settings.linflex1.value[0] == "Serial")?string?upper_case}
#define SPC5_SERIAL_USE_LINFLEX2            ${(conf.instance.linflex_settings.linflex2.value[0] == "Serial")?string?upper_case}
#define SPC5_SERIAL_LINFLEX0_PRIORITY       INTC_PSR_ENABLE(INTC_PSR_CORE2, ${conf.instance.irq_priority_settings.linflex0.value[0]})
#define SPC5_SERIAL_LINFLEX1_PRIORITY       INTC_PSR_ENABLE(INTC_PSR_CORE2, ${conf.instance.irq_priority_settings.linflex1.value[0]})
#define SPC5_SERIAL_LINFLEX2_PRIORITY       INTC_PSR_ENABLE(INTC_PSR_CORE2, ${conf.instance.irq_priority_settings.linflex2.value[0]})

#endif /* _MCUCONF_H_ */
