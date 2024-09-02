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

SET(ROOT ${CMSIS_PATH})

# Select which parts of the CMSIS-DSP must be compiled.
# There are some dependencies between the parts but they are not tracked
# by this cmake. So, enabling some functions may require to enable some
# other ones.
option(CONCATENATION        "Concatenation"         ON)
option(FULLYCONNECTED       "Fully Connected"       ON)
option(CONVOLUTION          "Convolutions"          ON)
option(ACTIVATION           "Activations"           ON)
option(POOLING              "Pooling"               ON)
option(SOFTMAX              "Softmax"               ON)
option(BASICMATHSNN         "Basic Maths for NN"    ON)
option(RESHAPE              "Reshape"               ON)
option(SVDF                 "SVDF"                  ON)

# When OFF it is the default behavior : all tables are included.
option(NNSUPPORT            "NN Support"            ON)


###########################
#
# CMSIS NN
#
###########################

# NN Sources
SET(NN ${ROOT}/CMSIS/NN)

list(APPEND CMAKE_MODULE_PATH ${NN}/Source)

add_library(cmsis-nn STATIC)

target_compile_options(cmsis-nn PRIVATE -Ofast)

### Includes
target_include_directories(cmsis-nn PUBLIC "${NN}/Include")
target_include_directories(cmsis-nn PUBLIC "${ROOT}/CMSIS/Core/Include")
target_include_directories(cmsis-nn PUBLIC "${ROOT}/CMSIS/DSP/Include")

if (BASICMATHSNN)
  add_subdirectory(BasicMathFunctions)
endif()

if (CONCATENATION)
  add_subdirectory(ConcatenationFunctions)
endif()

if (FULLYCONNECTED)
  add_subdirectory(FullyConnectedFunctions)
endif()

if (CONVOLUTION)
  add_subdirectory(ConvolutionFunctions)
endif()

if (ACTIVATION)
  add_subdirectory(ActivationFunctions)
endif()

if (POOLING)
  add_subdirectory(PoolingFunctions)
endif()

if (SOFTMAX)
  add_subdirectory(SoftmaxFunctions)
endif()

if (SVDF)
  add_subdirectory(SVDFunctions)
endif()

if (RESHAPE)
  add_subdirectory(ReshapeFunctions)
endif()

# Keep NNSUPPORT at the end
if (NNSUPPORT)
  add_subdirectory(NNSupportFunctions)
endif()
