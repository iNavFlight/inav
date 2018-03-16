/*
    Copyright (C) 2016 Jonathan Struebel

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
 * @file    common/shellcfg.h
 * @brief   CLI shell config header.
 *
 * @addtogroup SHELL
 * @{
 */

#ifndef SHELLCFG_H
#define SHELLCFG_H

#include "shell.h"

/*
 * Shell Thread size
 */
#define SHELL_WA_SIZE   THD_WORKING_AREA_SIZE(2048)

extern ShellConfig shell_cfg;

#endif  /* SHELLCFG_H */

/** @} */
