arm-none-eabi-gcc -c -g -I../../../../common_smp/inc -I../inc -mcpu=cortex-a7 sample_threadx.c
arm-none-eabi-gcc -c -g -mcpu=cortex-a7 startup.S
arm-none-eabi-gcc -c -g -mcpu=cortex-a7 MP_GIC.S
arm-none-eabi-gcc -c -g -mcpu=cortex-a7 MP_Mutexes.S
arm-none-eabi-gcc -c -g -mcpu=cortex-a7 v7.S
REM arm-none-eabi-ld -A cortex-a5 -T sample_threadx.ld reset.o crt0.o tx_initialize_low_level.o sample_threadx.o tx.a libc.a libgcc.a -o sample_threadx.out -M > sample_threadx.map
arm-none-eabi-gcc -T sample_threadx.ld -e Vectors -o sample_threadx.axf MP_GIC.o MP_Mutexes.o sample_threadx.o startup.o v7.o tx.a -Wl,-M > sample_threadx.map
