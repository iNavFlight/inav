set(CORTEX_M4F_COMMON_OPTIONS
    -mthumb
    -mcpu=cortex-m4
    -march=armv7e-m
    -mfloat-abi=hard
    -mfpu=fpv4-sp-d16
    -fsingle-precision-constant
    -Wdouble-promotion
)

set(CORTEX_M4F_COMPILE_OPTIONS
)

set(CORTEX_M4F_LINK_OPTIONS
)

set(CORTEX_M4F_DEFINITIONS
    __FPU_PRESENT=1
    ARM_MATH_CM4
    ARM_MATH_MATRIX_CHECK
    ARM_MATH_ROUNDING
    UNALIGNED_SUPPORT_DISABLE
)
