armasm -g --cpreproc --cpu=cortex-m0 --apcs=interwork tx_initialize_low_level.s
armcc -c -g --cpu=cortex-m0 -I../inc -I../../../../common/inc sample_threadx.c
armlink -d -o sample_threadx.axf --elf --map --ro-base=0x00000000 --rw-base=0x20000000 --first __tx_vectors --datacompressor=off --inline --info=inline --callgraph --list sample_threadx.map tx_initialize_low_level.o sample_threadx.o tx.a

