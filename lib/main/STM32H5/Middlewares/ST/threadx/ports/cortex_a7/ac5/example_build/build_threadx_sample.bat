armasm -g --cpu=cortex-a7.no_neon --fpu=softvfp --apcs=interwork tx_initialize_low_level.s
armcc -c -g --cpu=cortex-a7.no_neon --fpu=softvfp -I../../../../common/inc -I../inc sample_threadx.c
armlink -d -o sample_threadx.axf --elf --map --ro-base=0x00000000 --first tx_initialize_low_level.o(Init) --datacompressor=off --inline --info=inline --callgraph --list sample_threadx.map tx_initialize_low_level.o sample_threadx.o tx.a

