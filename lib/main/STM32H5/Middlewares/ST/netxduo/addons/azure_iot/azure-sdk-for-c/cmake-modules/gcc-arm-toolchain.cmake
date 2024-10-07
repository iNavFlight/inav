# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT
#
# Sets the cmake project as gcc-arm.
#

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
string(APPEND CMAKE_C_FLAGS_INIT " -mcpu=cortex-m4 -ffreestanding -nostdlib -mthumb")
