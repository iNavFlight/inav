#
# Copyright (c) 2019-2021 Arm Limited.
#
# SPDX-License-Identifier: Apache-2.0
#
# Licensed under the Apache License, Version 2.0 (the License); you may
# not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an AS IS BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

cmake_minimum_required(VERSION 3.15.6)

project(CMSISNN)

set(CMSIS_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../..")

option(BUILD_CMSIS_NN_FUNCTIONS "Build CMSIS-NN Source." ON)

if(BUILD_CMSIS_NN_FUNCTIONS)
    add_subdirectory(Source)
endif()
