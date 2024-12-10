# CMSIS NN
CMSIS NN software library is a collection of efficient neural network kernels developed to maximize the
performance and minimize the memory footprint of neural networks on Cortex-M processors.
## About
This page  give a quick overview of the functions available and key differences between them.

**Note:** The GitHub documentation does not follow the *develop* branch but rather the last official release in the *master* branch. Consequently, the group documentation linked to in the table table might not have the listed API. Please refer to the description in the [header](https://github.com/ARM-software/CMSIS_5/blob/develop/CMSIS/NN/Include/arm_nnfunctions.h) file instead.

## Support / Contact
For any questions or to reach the CMSIS-NN team, please create a new issue in https://github.com/ARM-software/CMSIS_5/issues
## Supported Framework
[TensorFlow Lite for Microcontrollers](https://www.tensorflow.org/lite/microcontrollers)
## Legacy vs TFL micro compliant APIs
There are two kinds of APIs available in the CMSIS-NN repository; One that supports a legacy symmetric quantization scheme[1] and one that supports TFL micro's symmetric quantization scheme. One of the main differences is how the quantization is performed. The legacy APIs have a fixed point format with power of 2 scaling. This simplifies the re-quantization to a cycle efficient shift operation. No new development is done on the legacy functions and all of the new development is on the functions that support TFL micro. The table below highlights some of the differences between the two formats for convolution related functions. The TFL micro compliant APIs in most cases have a _s8 suffix and is always specified in the API header file.

Operation | Legacy APIs | TFL micro compliant APIs|
|:-----------|:---------------------|:-------------|
Core loop | No input or filter offset | Input and/or filter offset |
Re-quantization | Shift and saturate in one instruction. ~ 5 cycles | Greater than 200 cycles for one output element
Quantization | Per layer quantization | Per-channel quantization
Output offset | No | Per-layer output offset
Fused Activation | No | Yes

## TFL micro compliant APIs
Group | API | Base Operator | Input Constraints | Additional memory required for <br/> optimizations (bytes) | DSP Optimized |  MVE Optimized | Other comments |
|:----| :---| :------------ | :---------------- | :--------------------------------------------------------| :-------------| :------------- | :------------- |
|[Conv](https://arm-software.github.io/CMSIS_5/NN/html/group__NNConv.html)||||| |  ||
||arm_convolve_wrapper_s8()|CONV| None |n.a.| Yes | Yes |The additional memory required depends on the optimal convolution function called.|
||arm_convolve_s8()|CONV| None |4 * (ker_x * ker_y * input_ch + delta)| Yes | Yes |delta - MVE only|
||arm_convolve_1x1_s8_fast() | CONV | dilation = 1 <br/> ker_x = 1, ker_y = 1 <br/> pad = 0<br/> stride = 1<br/> input_ch % 4 = 0| No | Yes |Yes ||
||arm_convolve_1_x_n_s8() | CONV | dilation = 1 <br/> output_y % 4 = 0 | 4 * ker_x * ker_y * input_ch |Yes |Yes||
|| arm_depthwise_conv_wrapper_s8()| DEPTHWISE_CONV | None |n.a.| Yes| Yes| The additional memory required depends on the optimal convolution function called|
|| arm_depthwise_conv_3x3_s8() | DEPTHWISE_CONV | dilation = 1 <br/> depth_multiplier = 1 <br/> pad_x <= 1 | No|No|No| Preferred function for 3x3 kernel size for DSP extension. </br> For MVE, use arm_depthwise_conv_s8_opt()||
| | arm_depthwise_conv_s8() | DEPTHWISE_CONV | None | No|No|No||
|| arm_depthwise_conv_s8_opt()| DEPTHWISE_CONV | dilation = 1 <br/> depth_multiplier = 1 | DSP: 2 * ker_x * ker_y * input_ch <br/> MVE: 2 * DSP + 4 | Yes| Yes| Best case is when channels are multiple of 4 or <br/>at the least >= 4 |
||arm_convolve_wrapper_s16()|CONV|None|n.a.| Yes | No |The additional memory required depends on the optimal convolution function called|
||arm_convolve_s16()|CONV|None|No| No | No ||
||arm_convolve_fast_s16()|CONV|dilation = 1, <br/> ker_x * ker_y * input_ch < 512 <br/> |4 * ker_x * ker_y * input_ch| Yes | No ||
| arm_depthwise_conv_s16() | DEPTHWISE_CONV | None | No|No|No||
|[Fully Connected](https://arm-software.github.io/CMSIS_5/NN/html/group__FC.html)||||| |  | |
|| arm_fully_connected_s8() |FULLY CONNECTED & <br/> MAT MUL  | None | No | Yes | Yes | |
|| arm_fully_connected_s16() |FULLY CONNECTED & <br/> MAT MUL  | None | No | Yes | No | |
|[Pooling](https://arm-software.github.io/CMSIS_5/NN/html/group__Pooling.html)||||| |  ||
|| arm_avgpool_s8() | AVERAGE POOL | None | input_ch * 2<br/>(DSP only) | Yes| Yes| Best case is when channels are multiple of 4 or <br/> at the least >= 4 |
|| arm_avgpool_s16() | AVERAGE POOL | None | None | No| No| Best case is when channels are multiple of 4 or <br/> at the least >= 4 |
|| arm_maxpool_s8() | MAX POOL | None | None | Yes| Yes|  |
|| arm_maxpool_s16() | MAX POOL | None | None | No| No|  |
|[Softmax](https://arm-software.github.io/CMSIS_5/NN/html/group__Softmax.html)||||| |  ||
||arm_softmax_q7()| SOFTMAX | None | None | Yes | No | Not bit exact to TFLu but can be up to 70x faster |
||arm_softmax_s8()| SOFTMAX | None | None | No | Yes | Bit exact to TFLu |
||arm_softmax_s8_s16()| SOFTMAX | None | None | No | No | Bit exact to TFLu |
||arm_softmax_s16()| SOFTMAX | None | None | No | No | Bit exact to TFLu |
||arm_softmax_u8()| SOFTMAX | None | None | No | No | Bit exact to TFLu |
|[SVDF](https://arm-software.github.io/CMSIS_5/NN/html/group__SVDF.html)||||| |  ||
||arm_svdf_s8()| SVDF | None | None | Yes | Yes | Bit exact to TFLu |
||arm_svdf_state_s16_s8()| SVDF | None | None | Yes | Yes | Bit exact to TFLu |
|[Misc](https://arm-software.github.io/CMSIS_5/NN/html/group__groupNN.html)||||| |  ||
||arm_reshape_s8()| SOFTMAX | None | None | No | No | |
||arm_elementwise_add_s8()| ELEMENTWISE ADD | None | None | Yes| Yes| Reshape is not done in this function <br/> Only minor improvements are expected |
||arm_elementwise_add_s16()| ELEMENTWISE ADD | None | None | No| No| Reshape is not done in this function <br/> Only minor improvements are expected |
||arm_elementwise_mul_s8()| ELEMENTWISE MUL | None | None | Yes| Yes| Reshape is not done in this function <br/> Only minor improvements are expected |
||arm_elementwise_mul_s16()| ELEMENTWISE MUL | None | None | No| No| Reshape is not done in this function <br/> Only minor improvements are expected |
||arm_relu_q7() | RELU | None | None | Yes| No|
||arm_relu6_s8() | RELU | None | None | Yes| No|
|[Concat](https://arm-software.github.io/CMSIS_5/NN/html/group__groupNN.html)||||| |  ||
||arm_concatenation_s8_w() | CONCAT | None | None | No| No||
||arm_concatenation_s8_x() | CONCAT | None | None | No| No||
||arm_concatenation_s8_y() | CONCAT | None | None | No| No||
||arm_concatenation_s8_z() | CONCAT | None | None | No| No||


## Building CMSIS-NN as a library
It is recommended to use toolchain files from [Arm Ethos-U Core Platform](https://review.mlplatform.org/admin/repos/ml/ethos-u/ethos-u-core-platform) project. These are supporting TARGET_CPU, which is a required argument. Note that if not specifying TARGET_CPU, these toolchains will set some default. The format must be TARGET_CPU=cortex-mXX, see examples below.
Clone Arm Ethos-U Core Platform project and build, for example:

```
cd </path/to/CMSIS_5>/CMSIS/NN
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=</path/to/ethos-u-core-platform>/cmake/toolchain/arm-none-eabi-gcc.cmake -DTARGET_CPU=cortex-m55
make
```

Some more examples, assuming Ethos-u-core-platform is cloned into your home directory:

```
cmake .. -DCMAKE_TOOLCHAIN_FILE=~/ethos-u-core-platform/cmake/toolchain/arm-none-eabi-gcc.cmake -DTARGET_CPU=cortex-m55
cmake .. -DCMAKE_TOOLCHAIN_FILE=~/ethos-u-core-platform/cmake/toolchain/arm-none-eabi-gcc.cmake -DTARGET_CPU=cortex-m7
cmake .. -DCMAKE_TOOLCHAIN_FILE=~/ethos-u-core-platform/cmake/toolchain/armclang.cmake -DTARGET_CPU=cortex-m3
```

### Compiler options
Default optimization level is Ofast. Please change according to project needs. Just bear in mind it will impact performance.

The compiler option '-fomit-frame-pointer' is enabled by default at -O and higher. With no optimization level you may need to specifiy '-fomit-frame-pointer' as a minimum.

The compiler option '-fno-builtin' does not utilize optimized implementations of e.g. memcpy and memset, which are heavily used by CMSIS-NN. It can significantly downgrade performance. So this should be avoided.
The compiler option '-ffreestanding' should also be avoided as it enables '-fno-builtin' implicitly.

## Reference
[1] Legacy CMSIS-NN and how to use it https://developer.arm.com/solutions/machine-learning-on-arm/developer-material/how-to-guides/converting-a-neural-network-for-arm-cortex-m-with-cmsis-nn/single-page
