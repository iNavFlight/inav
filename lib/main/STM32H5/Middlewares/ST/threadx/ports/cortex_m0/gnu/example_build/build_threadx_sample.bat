arm-none-eabi-gcc -c -g -mcpu=cortex-m0 -mthumb cortexm0_vectors.S
arm-none-eabi-gcc -c -g -mcpu=cortex-m0 -mthumb cortexm0_crt0.S
arm-none-eabi-gcc -c -g -mcpu=cortex-m0 -mthumb tx_initialize_low_level.S
arm-none-eabi-gcc -c -g -mcpu=cortex-m0 -mthumb -I../../../../common/inc -I../inc sample_threadx.c
arm-none-eabi-gcc -g -mcpu=cortex-m0 -mthumb -T sample_threadx.ld -ereset_handler -nostartfiles -o sample_threadx.out -Wl,-Map=sample_threadx.map cortexm0_vectors.o cortexm0_crt0.o tx_initialize_low_level.o sample_threadx.o tx.a
