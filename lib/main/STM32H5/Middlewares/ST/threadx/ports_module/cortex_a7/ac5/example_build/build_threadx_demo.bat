armasm -g --cpu=cortex-a7.no_neon --fpu=softvfp --apcs=interwork tx_initialize_low_level.s
armcc -g --cpu=cortex-a7.no_neon --fpu=softvfp -c -I../inc -I../../../../common/inc sample_threadx.c
armlink -d -o sample_threadx_module_manager.axf --elf --ro 0x80000000 --first tx_initialize_low_level.o(VECTORS) --remove --map --symbols --list sample_threadx.map tx_initialize_low_level.o sample_threadx.o tx.a
