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
 * SPC570Sxx drivers configuration.
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

#define SPC570Sxx_MCUCONF

/*
 * HAL driver system settings.
 */
#define SPC5_NO_INIT                        ${conf.instance.initialization_settings.do_not_init.value[0]?upper_case}
#define SPC5_ALLOW_OVERCLOCK                ${conf.instance.initialization_settings.allow_overclocking.value[0]?upper_case}
#define SPC5_DISABLE_WATCHDOG               ${conf.instance.initialization_settings.disable_watchdog.value[0]?upper_case}
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
#define SPC5_CGM_AC0_DC4_DIV_VALUE          ${conf.instance.initialization_settings.cgm_settings.ac0_dc4_divider.value[0]}
#define SPC5_CGM_AC0_DC5_DIV_VALUE          ${conf.instance.initialization_settings.cgm_settings.ac0_dc5_divider.value[0]}
#define SPC5_CGM_AC1_SC_BITS                SPC5_CGM_SC_${conf.instance.initialization_settings.cgm_settings.ac1_clock_source.value[0]}
#define SPC5_CGM_AC1_DC0_DIV_VALUE          ${conf.instance.initialization_settings.cgm_settings.ac1_dc0_divider.value[0]}
#define SPC5_CGM_AC2_SC_BITS                SPC5_CGM_SC_${conf.instance.initialization_settings.cgm_settings.ac2_clock_source.value[0]}
#define SPC5_CGM_AC3_SC_BITS                SPC5_CGM_SC_${conf.instance.initialization_settings.cgm_settings.ac3_clock_source.value[0]}
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
 * ICU driver system settings.
 */
#define SPC5_ICU_USE_SMOD0                  ${conf.instance.etimer_settings.etimer0_ch0.value[0]?upper_case}
#define SPC5_ICU_USE_SMOD1                  ${conf.instance.etimer_settings.etimer0_ch1.value[0]?upper_case}
#define SPC5_ICU_USE_SMOD2                  ${conf.instance.etimer_settings.etimer0_ch2.value[0]?upper_case}
#define SPC5_ICU_USE_SMOD3                  ${conf.instance.etimer_settings.etimer0_ch3.value[0]?upper_case}
#define SPC5_ICU_USE_SMOD4                  ${conf.instance.etimer_settings.etimer0_ch4.value[0]?upper_case}
#define SPC5_ICU_USE_SMOD5                  ${conf.instance.etimer_settings.etimer0_ch5.value[0]?upper_case}
#define SPC5_ICU_ETIMER0_PRIORITY           ${conf.instance.irq_priority_settings.etimer0.value[0]}

#define SPC5_ICU_USE_SMOD6                  ${conf.instance.etimer_settings.etimer1_ch0.value[0]?upper_case}
#define SPC5_ICU_USE_SMOD7                  ${conf.instance.etimer_settings.etimer1_ch1.value[0]?upper_case}
#define SPC5_ICU_USE_SMOD8                  ${conf.instance.etimer_settings.etimer1_ch2.value[0]?upper_case}
#define SPC5_ICU_USE_SMOD9                  ${conf.instance.etimer_settings.etimer1_ch3.value[0]?upper_case}
#define SPC5_ICU_USE_SMOD10                 ${conf.instance.etimer_settings.etimer1_ch4.value[0]?upper_case}
#define SPC5_ICU_USE_SMOD11                 ${conf.instance.etimer_settings.etimer1_ch5.value[0]?upper_case}
#define SPC5_ICU_ETIMER1_PRIORITY           ${conf.instance.irq_priority_settings.etimer1.value[0]}

#define SPC5_ICU_USE_SMOD12                 ${conf.instance.etimer_settings.etimer2_ch0.value[0]?upper_case}
#define SPC5_ICU_USE_SMOD13                 ${conf.instance.etimer_settings.etimer2_ch1.value[0]?upper_case}
#define SPC5_ICU_USE_SMOD14                 ${conf.instance.etimer_settings.etimer2_ch2.value[0]?upper_case}
#define SPC5_ICU_USE_SMOD15                 ${conf.instance.etimer_settings.etimer2_ch3.value[0]?upper_case}
#define SPC5_ICU_USE_SMOD16                 ${conf.instance.etimer_settings.etimer2_ch4.value[0]?upper_case}
#define SPC5_ICU_USE_SMOD17                 ${conf.instance.etimer_settings.etimer2_ch5.value[0]?upper_case}
#define SPC5_ICU_ETIMER2_PRIORITY           ${conf.instance.irq_priority_settings.etimer2.value[0]}

#define SPC5_ICU_USE_SMOD18                 ${conf.instance.etimer_settings.etimer3_ch0.value[0]?upper_case}
#define SPC5_ICU_USE_SMOD19                 ${conf.instance.etimer_settings.etimer3_ch1.value[0]?upper_case}
#define SPC5_ICU_USE_SMOD20                 ${conf.instance.etimer_settings.etimer3_ch2.value[0]?upper_case}
#define SPC5_ICU_USE_SMOD21                 ${conf.instance.etimer_settings.etimer3_ch3.value[0]?upper_case}
#define SPC5_ICU_USE_SMOD22                 ${conf.instance.etimer_settings.etimer3_ch4.value[0]?upper_case}
#define SPC5_ICU_USE_SMOD23                 ${conf.instance.etimer_settings.etimer3_ch5.value[0]?upper_case}
#define SPC5_ICU_ETIMER3_PRIORITY           ${conf.instance.irq_priority_settings.etimer3.value[0]}

/*
 * SERIAL driver system settings.
 */
#define SPC5_SERIAL_USE_LINFLEX0            ${(conf.instance.linflex_settings.linflex0.value[0] == "Serial")?string?upper_case}
#define SPC5_SERIAL_USE_LINFLEX1            ${(conf.instance.linflex_settings.linflex1.value[0] == "Serial")?string?upper_case}
#define SPC5_SERIAL_LINFLEX0_PRIORITY       INTC_PSR_ENABLE(INTC_PSR_CORE0, ${conf.instance.irq_priority_settings.linflex0.value[0]})
#define SPC5_SERIAL_LINFLEX1_PRIORITY       INTC_PSR_ENABLE(INTC_PSR_CORE0, ${conf.instance.irq_priority_settings.linflex1.value[0]})
/*
 * CAN driver system settings.
 */
#define SPC5_CAN_USE_FILTERS                ${conf.instance.flexcan_settings.flexcan_enable_filters.value[0]?upper_case}

#define SPC5_CAN_USE_FLEXCAN0               ${conf.instance.flexcan_settings.flexcan0.value[0]?upper_case}
#define SPC5_CAN_FLEXCAN0_USE_EXT_CLK       ${conf.instance.flexcan_settings.flexcan0_use_external_clock.value[0]?upper_case}
#define SPC5_CAN_FLEXCAN0_PRIORITY          ${conf.instance.irq_priority_settings.flexcan0.value[0]}
#define SPC5_CAN_NUM_RX_MAILBOXES			${conf.instance.flexcan_settings.can_configurations.mailboxes_configuration.number_of_rx_mailboxes.value[0]}
#define SPC5_CAN_NUM_TX_MAILBOXES			${conf.instance.flexcan_settings.can_configurations.mailboxes_configuration.number_of_tx_mailboxes.value[0]}
#define SPC5_CAN_FLEXCAN0_START_PCTL        (SPC5_ME_PCTL_RUN(1) |          \
                                             SPC5_ME_PCTL_LP(2))
#define SPC5_CAN_FLEXCAN0_STOP_PCTL         (SPC5_ME_PCTL_RUN(0) |          \
                                             SPC5_ME_PCTL_LP(0))

#endif /* _MCUCONF_H_ */
