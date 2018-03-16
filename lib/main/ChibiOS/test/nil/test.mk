# List of all the ChibiOS/RT test files.
TESTSRC = ${CHIBIOS}/test/lib/ch_test.c \
          ${CHIBIOS}/test/nil/test_root.c \
          ${CHIBIOS}/test/nil/test_sequence_001.c \
          ${CHIBIOS}/test/nil/test_sequence_002.c

# Required include directories
TESTINC = ${CHIBIOS}/test/lib \
          ${CHIBIOS}/test/nil
