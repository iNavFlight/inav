# Zephyr Test Plan

The following roughly describes how mcuboot is tested on Zephyr.  The
testing is done with the code in `samples/zephyr`.  These examples
were written using the FRDM-K64F, but other boards should be similar.
At this time, however, the partitions are hardcoded in the Makefile
targets to flash.

Note that the script "run-tests.sh" in that directory is helpful for
automating the process, and provides simple "y or n" prompts for each
test case and expected result.

## Building and running.

The tests are build using the various `test-*` targets in
`samples/zephyr/Makefile`.  For each test, invoke `make` with that
target:

    $ make test-good-rsa

Begin by doing a full erase, and programming the bootloader itself:

    $ pyocd erase --chip
    $ make flash_boot

After it resets, look for "main: Starting bootloader", a few debug
messages, and lastly: "main: Unable to find bootable image".

Then, load hello1:

    $ make flash_hello1

This should print "main: Jumping to the first image slot", and you
should get an image "hello1".

Note that there are comments with each test target describing the
intended behavior for each of these steps.  Sometimes an upgrade will
happen and sometimes it will not.

    $ make flash_hello2

This should print a message: `boot_swap_type: Swap type: test`, and
you should see "hello2".

Now reset the target::

    $ pyocd commander -c reset

And you should see a revert and "hello1" running.

## Testing that mark ok works

Repeat this, to make sure we can mark the image as OK, and that a
revert doesn't happen:

    $ make flash_hello1
    $ make flash_hello2

We should have just booted the hello2.  Mark this as OK:

    $ pyocd flash -a 0x7ffe8 image_ok.bin
    $ pyocd commander -c reset

And make sure this stays in the "hello2" image.

This step doesn't make sense on the tests where the upgrade doesn't
happen.

## Testing all configurations

Repeat these steps for each of the `test-*` targest in the Makefile.
