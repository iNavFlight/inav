#
# Copyright (c) 2019-2022 Arm Limited.
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

file(GLOB SRC "./*_s8.c")
target_sources(cmsis-nn PRIVATE ${SRC} arm_q7_to_q15_with_offset.c
                                       arm_nn_mat_mul_kernel_s16.c
                                       arm_q7_to_q15_with_offset.c
                                       arm_nn_mat_mul_kernel_s16.c
                                       arm_nn_vec_mat_mult_t_s16.c
                                       arm_q7_to_q15_no_shift.c)

