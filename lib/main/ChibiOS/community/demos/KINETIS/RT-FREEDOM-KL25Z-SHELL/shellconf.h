/*
    ChibiOS - Copyright (C) 2006..2016 Giovanni Di Sirio

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
 * @file    shellconf.h
 * @brief   Simple CLI shell configuration header.
 *
 * @addtogroup SHELL
 * @{
 */

#ifndef SHELLCONF_H
#define SHELLCONF_H

/**
 * @brief   Shell maximum input line length.
 */
#if !defined(SHELL_MAX_LINE_LENGTH) || defined(__DOXYGEN__)
#define SHELL_MAX_LINE_LENGTH       64
#endif

/**
 * @brief   Shell maximum arguments per command.
 */
#if !defined(SHELL_MAX_ARGUMENTS) || defined(__DOXYGEN__)
#define SHELL_MAX_ARGUMENTS         4
#endif

/**
 * @brief   Shell maximum command history.
 */
#if !defined(SHELL_MAX_HIST_BUFF) || defined(__DOXYGEN__)
#define SHELL_MAX_HIST_BUFF         8 * SHELL_MAX_LINE_LENGTH
#endif

/**
 * @brief   Enable shell command history
 */
#if !defined(SHELL_USE_HISTORY) || defined(__DOXYGEN__)
#define SHELL_USE_HISTORY           TRUE
#endif

/**
 * @brief   Enable shell command completion
 */
#if !defined(SHELL_USE_COMPLETION) || defined(__DOXYGEN__)
#define SHELL_USE_COMPLETION        TRUE
#endif

/**
 * @brief   Shell Maximum Completions (Set to max commands with common prefix)
 */
#if !defined(SHELL_MAX_COMPLETIONS) || defined(__DOXYGEN__)
#define SHELL_MAX_COMPLETIONS       8
#endif

/**
 * @brief   Enable shell escape sequence processing
 */
#if !defined(SHELL_USE_ESC_SEQ) || defined(__DOXYGEN__)
#define SHELL_USE_ESC_SEQ           TRUE
#endif

/*===========================================================================*/
/* Shell command settings                                                    */
/*===========================================================================*/

/**
 * @brief   Enable shell exit command
 */
#if !defined(SHELL_CMD_EXIT_ENABLED) || defined(__DOXYGEN__)
#define SHELL_CMD_EXIT_ENABLED              TRUE
#endif

/**
 * @brief   Enable shell info command
 */
#if !defined(SHELL_CMD_INFO_ENABLED) || defined(__DOXYGEN__)
#define SHELL_CMD_INFO_ENABLED              TRUE
#endif

/**
 * @brief   Enable shell echo command
 */
#if !defined(SHELL_CMD_ECHO_ENABLED) || defined(__DOXYGEN__)
#define SHELL_CMD_ECHO_ENABLED              TRUE
#endif

/**
 * @brief   Enable shell systime command
 */
#if !defined(SHELL_CMD_SYSTIME_ENABLED) || defined(__DOXYGEN__)
#define SHELL_CMD_SYSTIME_ENABLED           TRUE
#endif

/**
 * @brief   Enable shell mem command
 */
#if !defined(SHELL_CMD_MEM_ENABLED) || defined(__DOXYGEN__)
#define SHELL_CMD_MEM_ENABLED               TRUE
#endif

/**
 * @brief   Enable shell threads command
 */
#if !defined(SHELL_CMD_THREADS_ENABLED) || defined(__DOXYGEN__)
#define SHELL_CMD_THREADS_ENABLED           TRUE
#endif

/**
 * @brief   Enable shell test command
 */
#if !defined(SHELL_CMD_TEST_ENABLED) || defined(__DOXYGEN__)
#define SHELL_CMD_TEST_ENABLED              TRUE
#endif

/**
 * @brief   Define test thread working area
 */
#if !defined(SHELL_CMD_TEST_WA_SIZE) || defined(__DOXYGEN__)
#define SHELL_CMD_TEST_WA_SIZE              THD_WORKING_AREA_SIZE(256)
#endif

#endif /* SHELLCONF_H */

/** @} */
