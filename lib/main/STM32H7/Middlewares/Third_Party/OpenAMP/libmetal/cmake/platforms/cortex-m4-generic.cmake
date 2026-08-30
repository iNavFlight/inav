set (CMAKE_SYSTEM_PROCESSOR "arm" CACHE STRING "")
set (MACHINE "cortexm" CACHE STRING "")

set (CMAKE_C_FLAGS "-mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Og -fmessage-length=0 -ffunction-sections -c" CACHE STRING "")

include (cross-generic-gcc)

# vim: expandtab:ts=2:sw=2:smartindent
