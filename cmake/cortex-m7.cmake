set(CORTEX_M7_COMMON_OPTIONS
    -mthumb
    -mcpu=cortex-m7
    -mfloat-abi=hard
    -mfpu=fpv5-sp-d16
    -fsingle-precision-constant
    -Wdouble-promotion
)

set(CORTEX_M7_COMPILE_OPTIONS
)

set(CORTEX_M7_LINK_OPTIONS
)

set(CORTEX_M7_DEFINITIONS
    __FPU_PRESENT=1
    ARM_MATH_CM7
    ARM_MATH_MATRIX_CHECK
    ARM_MATH_ROUNDING
    UNALIGNED_SUPPORT_DISABLE
)
