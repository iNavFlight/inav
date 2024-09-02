#
# Copyright (c) 2019-2021 Arm Limited. All rights reserved.
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

project(CMSISNNSoftmax)

file(GLOB SRC "./*_s8.c")
add_library(CMSISNNSoftmax STATIC ${SRC})

### Includes
target_include_directories(CMSISNNSoftmax PUBLIC "${NN}/Include")
target_include_directories(CMSISNNSoftmax PUBLIC "${ROOT}/CMSIS/Core/Include")
target_include_directories(CMSISNNSoftmax PUBLIC "${ROOT}/CMSIS/DSP/Include")



