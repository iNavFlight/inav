# This is called from the targets that build the
# openocd.cfg file
if(NOT CMAKE_ARGC EQUAL 6)
    message(FATAL_ERROR "usage: cmake -P openocd_cfg.cmake <target> <interface> <output>")
endif()

set(OPENOCD_TARGET ${CMAKE_ARGV3})
set(OPENOCD_INTERFACE ${CMAKE_ARGV4})
set(OUTPUT ${CMAKE_ARGV5})

set(opencd_cfg)
list(APPEND openocd_cfg "source [find interface/${OPENOCD_INTERFACE}.cfg]")
list(APPEND openocd_cfg "source [find target/${OPENOCD_TARGET}.cfg]")
list(APPEND openocd_cfg "init")
list(APPEND openocd_cfg "arm semihosting enable")
list(APPEND openocd_cfg "reset halt")
list(JOIN openocd_cfg "\n" contents)
set(contents "${contents}\n")

file(WRITE ${OUTPUT} ${contents})