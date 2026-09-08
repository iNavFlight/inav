set(CORTEX_M33_COMMON_OPTIONS
    -mthumb
    -mcpu=cortex-m33
    -march=armv8-m.main+fp+dsp
    -mfloat-abi=hard
    -mfpu=fpv5-sp-d16
    -fsingle-precision-constant
    -Wdouble-promotion
)

set(CORTEX_M33_COMPILE_OPTIONS
)

set(CORTEX_M33_LINK_OPTIONS
)

set(CORTEX_M33_DEFINITIONS
    __FPU_PRESENT=1
    ARM_MATH_CM4
    ARM_MATH_MATRIX_CHECK
    ARM_MATH_ROUNDING
    UNALIGNED_SUPPORT_DISABLE
)
