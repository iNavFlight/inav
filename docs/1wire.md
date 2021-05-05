# 1-wire passthrough esc programming

### ESCs must have the BlHeli Bootloader.

If your ESCs didn't come with BlHeli Bootloader, you'll need to flash them with an ArduinoISP programmer first. [Here's a guide](http://bit.ly/blheli-f20).

This is the option you need to select for the bootloader:

![Flashing BlHeli Bootloader](assets/images/blheli-bootloader.png)

Currently supported on all boards with at least 128kB of flash memory (all F3, F4 and F7).

## Usage

  - Plug in the USB cable and connect to your board with the INAV configurator.

  - Open the BlHeli Suite.

  - Ensure you have selected the correct Atmel or SILABS "Cleanflight" option under the "Select ATMEL / SILABS Interface" menu option.

  - Ensure you have port for your external USB/UART adapter selected, if you're using one, otherwise pick the same COM port that you normally use for INAV.

  - Click "Connect" and wait for the connection to complete. If you get a COM error, hit connect again. It will probably work.

  - Use the boxes at the bottom to select the ESCs you have connected. Note that the boxes correspond directly to the ports on your flight controller. For example if you have motors on ports 1-4, pick boxes 1-4 or in the case of a tri-copter that uses motors on ports 3, 4 and 5, select those ports in BlHeli.

  - Click "Read Setup"

  - Use BlHeli suite as normal.

  - When you're finished with one ESC, click "Disconnect"

## Implementing and Configuring targets

The following parameters can be used to enable and configure this in the related target.h file:

    USE_SERIAL_1WIRE              Enables the 1wire code, defined in target.h


  - For new targets

    - in `target.h`

        ```
        // Turn on serial 1wire passthrough
        #define USE_SERIAL_1WIRE
        // How many escs does this board support?
        #define ESC_COUNT 6
        // STM32F3DISCOVERY TX - PC3 connects to UART RX
        #define S1W_TX_GPIO         GPIOC
        #define S1W_TX_PIN          GPIO_Pin_3
        // STM32F3DISCOVERY RX - PC1 connects to UART TX
        #define S1W_RX_GPIO         GPIOC
        #define S1W_RX_PIN          GPIO_Pin_1
        ```

    - in `serial_1wire.c`

       ```
       // Define your esc hardware
       #if defined(STM32F3DISCOVERY)
       const escHardware_t escHardware[ESC_COUNT] = {
         { GPIOD, 12 },
         { GPIOD, 13 },
         { GPIOD, 14 },
         { GPIOD, 15 },
         { GPIOA, 1 },
         { GPIOA, 2 }
       };
       ```

## Development Notes

On the STM32F3DISCOVERY, an external pullup on the ESC line may be necessary. I needed a 3v, 4.7k pullup.
