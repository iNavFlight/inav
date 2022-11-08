#!/bin/sh

ed $1 << EOF
    /BMI270_EXTI_PIN/d
    /ENSURE_MPU_DATA_READY_IS_LOW/d
    /GYRO_1_EXTI_PIN/d
    /GYRO_2_EXTI_PIN/d
    /GYRO_INT_EXTI/d
    /MPU6000_EXTI_PIN/d
    /MPU6500_1_EXTI_PIN/d
    /MPU6500_EXTI_PIN/d
    w
    q
EOF


