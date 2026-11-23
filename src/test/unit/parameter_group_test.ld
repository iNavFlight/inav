/* Host tests linking parameter_group.c need the PG section boundaries that
 * firmware linker scripts normally provide. Mach-O supplies these automatically. */
SECTIONS
{
    .pg_registry : ALIGN(8)
    {
        __pg_registry_start = .;
        KEEP(*(.pg_registry))
        __pg_registry_end = .;
    }
    .pg_resetdata : ALIGN(2)
    {
        __pg_resetdata_start = .;
        KEEP(*(.pg_resetdata))
        __pg_resetdata_end = .;
    }
}
INSERT AFTER .data;
