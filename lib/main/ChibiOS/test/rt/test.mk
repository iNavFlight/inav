# List of all the ChibiOS/RT test files.
TESTSRC = ${CHIBIOS}/test/rt/test.c \
          ${CHIBIOS}/test/rt/testthd.c \
          ${CHIBIOS}/test/rt/testsem.c \
          ${CHIBIOS}/test/rt/testmtx.c \
          ${CHIBIOS}/test/rt/testmsg.c \
          ${CHIBIOS}/test/rt/testmbox.c \
          ${CHIBIOS}/test/rt/testevt.c \
          ${CHIBIOS}/test/rt/testheap.c \
          ${CHIBIOS}/test/rt/testpools.c \
          ${CHIBIOS}/test/rt/testdyn.c \
          ${CHIBIOS}/test/rt/testqueues.c \
          ${CHIBIOS}/test/rt/testsys.c \
          ${CHIBIOS}/test/rt/testbmk.c

# Required include directories
TESTINC = ${CHIBIOS}/test/rt
