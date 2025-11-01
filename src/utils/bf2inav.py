#
# This script can be used to Generate a basic working target from a Betaflight Configuration.
# The idea is that this target can be used as a starting point for full INAV target.
#
# The generated target will often be enough to get INAV working, but may need some manual editing.
#
# Betaflight Configuration files are available at https://github.com/betaflight/config
#
# Common things to look for: target.c timer definitions. You may need to change timers around
# to get all features working. The script will add commented out lines for all timer possibilites
# for a given pin.


import sys
import os
import io
import getopt
import re
import json
import random
import string
import yaml

version = '0.1'

def translateFunctionName(bffunction, index):
    return bffunction + '_' + index

def translatePin(bfpin):
    pin = re.sub(r'^([A-Z])0*(\d+)$', r'P\1\2', bfpin)
    return pin

def mcu2target(mcu):
#mcu STM32F405
    if mcu['type'] == 'STM32F405':
        return 'target_stm32f405xg'

#mcu STM32F411
    if mcu['type'] == 'STM32F411':
        return 'target_stm32f411xe'
    
#mcu STM32F7X2
    if mcu['type'] == 'STM32F7X2':
        return 'target_stm32f722xe'
    
#mcu STM32F745
    if mcu['type'] == 'STM32F745':
        return 'target_stm32f745xg'

#mcu STM32H743
    if mcu['type'] == 'STM32H743':
        return 'target_stm32h743xi'

#mcu 'AT32F435G'
    if mcu['type'] == 'AT32F435G':
        return 'target_at32f43x_xGT7'

#mcu 'AT32F435M'
    if mcu['type'] == 'AT32F435M':
        return 'target_at32f43x_xMT7'
    
    print("Unknown MCU: %s!" % (mcu))
    sys.exit(-1)

def getPortConfig(map):
    mcu = map['mcu']
#mcu STM32F405
    if mcu['type'] == 'STM32F405':
        return """
#define TARGET_IO_PORTA         0xffff
#define TARGET_IO_PORTB         0xffff
#define TARGET_IO_PORTC         0xffff
#define TARGET_IO_PORTD         0xffff
#define TARGET_IO_PORTE         0xffff
#define TARGET_IO_PORTF         0xffff

"""

#mcu STM32F411
    if mcu['type'] == 'STM32F411':
        return """
#define TARGET_IO_PORTA         0xffff
#define TARGET_IO_PORTB         0xffff
#define TARGET_IO_PORTC         0xffff
#define TARGET_IO_PORTD         0xffff
#define TARGET_IO_PORTE         0xffff
#define TARGET_IO_PORTF         0xffff
"""
    
#mcu STM32F7X2
    if mcu['type'] == 'STM32F7X2':
        return """
#define TARGET_IO_PORTA         0xffff
#define TARGET_IO_PORTB         0xffff
#define TARGET_IO_PORTC         0xffff
#define TARGET_IO_PORTD         0xffff
#define TARGET_IO_PORTE         0xffff
#define TARGET_IO_PORTF         0xffff
"""
    
#mcu STM32F745
    if mcu['type'] == 'STM32F745':
        return """
#define TARGET_IO_PORTA         0xffff
#define TARGET_IO_PORTB         0xffff
#define TARGET_IO_PORTC         0xffff
#define TARGET_IO_PORTD         0xffff
#define TARGET_IO_PORTE         0xffff
#define TARGET_IO_PORTF         0xffff
"""

#mcu STM32H743
    if mcu['type'] == 'STM32H743':
        return """
#define TARGET_IO_PORTA         0xffff
#define TARGET_IO_PORTB         0xffff
#define TARGET_IO_PORTC         0xffff
#define TARGET_IO_PORTD         0xffff
#define TARGET_IO_PORTE         0xffff
#define TARGET_IO_PORTF         0xffff
#define TARGET_IO_PORTG         0xffff
"""

#mcu 'AT32F435G'
    if mcu['type'] == 'AT32F435G':
        return """#define TARGET_IO_PORTA         0xffff
#define TARGET_IO_PORTB         0xffff
#define TARGET_IO_PORTC         0xffff
#define TARGET_IO_PORTD         0xffff
#define TARGET_IO_PORTH         0xffff
"""

#mcu 'AT32F435M'
    if mcu['type'] == 'AT32F435M':
        return """#define TARGET_IO_PORTA         0xffff
#define TARGET_IO_PORTB         0xffff
#define TARGET_IO_PORTC         0xffff
#define TARGET_IO_PORTD         0xffff
#define TARGET_IO_PORTH         0xffff
"""
    
    print("Unknown MCU: %s" % (mcu))
    sys.exit(-1)

def writeCmakeLists(outputFolder, map):
    file = open(outputFolder + '/CMakeLists.txt', "w+")

    t = mcu2target(map['mcu'])

    file.write("%s(%s SKIP_RELEASES)\n" % (t, map['board_name']))

    return


def findPinsByFunction(function, map):
    result = []
    for func in map['funcs']:
        pattern = r"^%s" % (function)
        if re.search(pattern, func):
            #print ("%s: %s" % (function, func))
            result.append(map['funcs'][func])
    
    return result

def findPinByFunction(function, map):
    if function in map['funcs']:
        return map['funcs'][function]

    return None


def getPwmOutputCount(map):
    motors = findPinsByFunction("MOTOR", map)
    servos = findPinsByFunction("SERVO", map)

    return len(motors) + len(servos)

def getGyroAlign(map):
    bfalign = map['defines'].get('GYRO_1_ALIGN', 'CW0_DEG')
    return bfalign
    #m = re.search(r"^CW(\d+)(FLIP)?$", bfalign)
    #if m:
    #    deg = m.group(1)
    #    flip = m.group(2)
    #    if flip:
    #        return "CW%s_DEG_FLIP" % (deg)
    #    else:
    #        return "CW%s_DEG" % (deg)

def getSerialByFunction(map, function):
    for serial in map.get("serial"):
        if map['serial'][serial].get('FUNCTION') == function:
            return serial

    return None

def getSerialMspDisplayPort(map):
    return getSerialByFunction(map, "131072")

def getSerialRx(map):
    rx = getSerialByFunction(map, "64")
    if(rx != None):
        return int(rx) + 1
    return None

def writeTargetH(folder, map):
    file = open(folder + '/target.h', "w+")

    file.write("""/*
 * This file is part of INAV.
 *
 * INAV is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * INAV is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with INAV.  If not, see <http://www.gnu.org/licenses/>.
 *
 * This target has been autgenerated by bf2inav.py
 */

#pragma once

//#define USE_TARGET_CONFIG

#define DEFAULT_FEATURES        (FEATURE_OSD | FEATURE_CURRENT_METER | FEATURE_VBAT | FEATURE_TELEMETRY  )


 \n"""
 )
    board_id = ''.join(random.choice(string.ascii_uppercase) for i in range(4))
    file.write("#define TARGET_BOARD_IDENTIFIER \"%s\"\n" % (board_id))
    file.write("#define USBD_PRODUCT_STRING \"%s\"\n" % (map['board_name']))

    # beeper
    file.write("// Beeper\n")
    pin = findPinByFunction('BEEPER', map)
    #print ("BEEPER")
    if pin:
        #print ("BEEPER: %s" % (pin))
        file.write("#define USE_BEEPER\n")
        file.write("#define BEEPER %s\n" % (pin))
        if 'BEEPER_INVERTED' in map['empty_defines']:
            file.write("#define BEEPER_INVERTED\n")
            #print ("INVERTED")
    
    # Leds
    file.write("// Leds\n")
    pin = findPinByFunction('LED_STRIP', map)
    #print ("LED")
    if pin:
        #print ("LED: %s" % (pin))
        file.write('#define USE_LED_STRIP\n')
        file.write("#define WS2811_PIN %s\n" % (pin))

    for i in range(0, 9):
        pin = findPinByFunction("LED%i" % (i), map)
        if pin:
            #print ("LED%i: %s" % (i, pin))
            file.write("#define LED%i %s\n" % (i, pin))

    # Serial ports and usb
    #print ("SERIAL")
    file.write("// UARTs\n")
    file.write("#define USB_IO\n")
    file.write("#define USE_VCP\n")
    serial_count = 1 
    pin = findPinByFunction('USB_DETECT', map)
    if pin:
        file.write("#define USE_USB_DETECT\n")
        file.write("#define USB_DETECT_PIN %s\n" % (pin))
        #file.write("#define VBUS_SENSING_ENABLED\n");  
 
    for i in range(1, 9):
        txpin = findPinByFunction("UART%i_TX" % (i), map)
        rxpin = findPinByFunction("UART%i_RX" % (i), map)
        if txpin or rxpin:
            #print ("UART%s" % (i))
            file.write("#define USE_UART%i\n" % (i))
            serial_count+=1
        else:
            continue

        if rxpin:
            file.write("#define UART%i_RX_PIN %s\n" % (i, rxpin))
        if txpin:
            file.write("#define UART%i_TX_PIN %s\n" % (i, txpin))
        else:
            file.write("#define UART%i_TX_PIN %s\n" % (i, rxpin))

    # soft serial
    for i in range(1, 9):
        txpin = findPinByFunction("SOFTSERIAL%i_TX" % (i), map)
        rxpin = findPinByFunction("SOFTSERIAL%i_RX" % (i), map)
        idx = i
        if txpin != None or rxpin != None:
            #print ("SOFTUART%s" % (i))
            file.write("#define USE_SOFTSERIAL%i\n" % (idx))
            serial_count+=1
        else:
            continue

        if txpin != None:
            file.write("#define SOFTSERIAL_%i_TX_PIN %s\n" % (idx, txpin))
        else:
            file.write("#define SOFTSERIAL_%i_TX_PIN %s\n" % (idx, rxpin))
    
        if rxpin != None:
            file.write("#define SOFTSERIAL_%i_RX_PIN %s\n" % (idx, rxpin))
        else:
            file.write("#define SOFTSERIAL_%i_RX_PIN %s\n" % (idx, txpin))
   
    file.write("#define SERIAL_PORT_COUNT %i\n" % (serial_count))

    file.write("#define DEFAULT_RX_TYPE RX_TYPE_SERIAL\n")
    file.write("#define SERIALRX_PROVIDER SERIALRX_CRSF\n")

    # TODO: map default serial uart
    #serial_rx = getSerialRx(map)
    serial_rx = None

    if serial_rx != None:
        file.write("#define SERIALRX_UART SERIAL_PORT_USART%s\n" % (serial_rx))

    file.write("// SPI\n")
    use_spi_defined = False
    for i in range(1, 9):
        sckpin = findPinByFunction("SPI%i_SCK" % (i), map)
        misopin = findPinByFunction("SPI%i_SDI" % (i), map)
        mosipin = findPinByFunction("SPI%i_SDO" % (i), map)
        if (sckpin or misopin or mosipin):
            if (not use_spi_defined):
                use_spi_defined = True
                file.write("#define USE_SPI\n")
            file.write("#define USE_SPI_DEVICE_%i\n" % (i))
        
        if sckpin:
            file.write("#define SPI%i_SCK_PIN %s\n" % (i, sckpin))
        if misopin:
            file.write("#define SPI%i_MISO_PIN %s\n" % (i, misopin))
        if mosipin:
            file.write("#define SPI%i_MOSI_PIN %s\n" % (i, mosipin))

    use_i2c_defined = False
    for i in range(1, 9):
        sclpin = findPinByFunction("I2C%i_SCL" % (i), map)
        sdapin = findPinByFunction("I2C%i_SDA" % (i), map)
        if (sclpin or sdapin):
            if (not use_i2c_defined):
                file.write("// I2C\n")
                #print ("I2C")
                use_i2c_defined = True
                file.write("#define USE_I2C\n")
            file.write("#define USE_I2C_DEVICE_%i\n" % (i))
        
        if sclpin:
            file.write("#define I2C%i_SCL %s\n" % (i, sclpin))
        if sdapin:
            file.write("#define I2C%i_SDA %s\n" % (i, sdapin))
    
    if 'MAG_I2C_INSTANCE' in map['defines']:
        file.write("// MAG\n")
        bfinstance = map['defines']['MAG_I2C_INSTANCE']
        file.write("#define USE_MAG\n")
        file.write("#define USE_MAG_ALL\n")
        # (I2CDEV_1)
        m = re.search(r'^\s*#define\s+MAG_I2C_INSTANCE\s+\(?I2CDEV_(\d+)\)?\s*$', bfinstance)
        if m:
            file.write("#define MAG_I2C_BUS BUS_I2C%i" % (m.group(1)))

    file.write("// ADC\n")


    # ADC_BATT ch1
    use_adc = False
    pin = findPinByFunction('ADC_VBAT', map)
    if pin:
        use_adc = True
        file.write("#define ADC_CHANNEL_1_PIN %s\n" % (pin))
        file.write("#define VBAT_ADC_CHANNEL ADC_CHN_1\n");
    
    # ADC_CURR ch2
    pin = findPinByFunction('ADC_CURR', map)
    if pin:
        use_adc = True
        file.write("#define ADC_CHANNEL_2_PIN %s\n" % (pin))
        file.write("#define CURRENT_METER_ADC_CHANNEL ADC_CHN_2\n");
    # ADC_RSSI ch3
    pin = findPinByFunction('ADC_RSSI', map)
    if pin:
        use_adc = True
        file.write("#define ADC_CHANNEL_3_PIN %s\n" % (pin))
        file.write("#define RSSI_ADC_CHANNEL ADC_CHN_3\n");

    # ADC_EXT  ch4 (airspeed?)
    pin = findPinByFunction('ADC_EXT_1', map)
    if pin:
        use_adc = True
        file.write("#define ADC_CHANNEL_4_PIN %s\n" % (pin))
        file.write("#define AIRSPEED_ADC_CHANNEL ADC_CHN_4\n");

    if use_adc:
        file.write("#define USE_ADC\n")
        file.write("#define ADC_INSTANCE ADC1\n")
    # TODO:
    #define ADC1_DMA_STREAM             DMA2_Stream4

    file.write("// Gyro & ACC\n")
    for supportedgyro in ['BMI160', 'BMI270', 'ICM20689', 'ICM42605', 'MPU6000', 'MPU6500', 'MPU9250']:
        found = False
        for var in ['USE_ACCGYRO_', 'USE_ACC_', 'USE_ACC_SPI', 'USE_GYRO_', 'USE_GYRO_SPI_']:
                val = var + supportedgyro
                if val in map['empty_defines']:
                    found = True
                    break
        
        if found:
            #print (supportedgyro)
            file.write("#define USE_IMU_%s\n" % (supportedgyro))
            file.write("#define %s_CS_PIN       %s\n" % (supportedgyro, findPinByFunction('GYRO_1_CS', map)))
            file.write("#define %s_SPI_BUS BUS_%s\n" % (supportedgyro, map['defines']['GYRO_1_SPI_INSTANCE']))
            file.write("#define IMU_%s_ALIGN    %s\n" % (supportedgyro, getGyroAlign(map)))


    if 'USE_BARO' in map['empty_defines']:
        #print ("BARO")
        file.write("// BARO\n")
        file.write("#define USE_BARO\n")
        if 'BARO_I2C_INSTANCE' in map['defines']:
            file.write("#define USE_BARO_ALL\n")
            m = re.search(r'I2CDEV_(\d+)', map['defines']['BARO_I2C_INSTANCE'])
            if m:
                file.write("#define BARO_I2C_BUS BUS_I2C%s\n" % (m.group(1)))
        if 'BARO_SPI_INSTANCE' in map['defines']:
            file.write("#define USE_BARO_BMP280\n")
            file.write("#define USE_BARO_SPI_BMP280\n")
            file.write("#define BMP280_SPI_BUS BUS_%s\n" % (map['defines']['BARO_SPI_INSTANCE']))
            file.write("#define BMP280_CS_PIN %s\n" % (findPinByFunction('BARO_CS', map)))

    file.write("// OSD\n")
    if 'USE_MAX7456' in map['empty_defines']:
        #print ("ANALOG OSD")
        file.write("#define USE_MAX7456\n")
        pin = findPinByFunction('MAX7456_SPI_CS', map)
        file.write("#define MAX7456_CS_PIN %s\n" % (pin))
        file.write("#define MAX7456_SPI_BUS BUS_%s\n" % (map['defines']['MAX7456_SPI_INSTANCE']))
    file.write("// Blackbox\n")

    # Flash:
    if 'USE_FLASH' in map['empty_defines']:
        #print ("FLASH BLACKBOX")
        cs = findPinByFunction("FLASH_CS", map)
        spiflash_bus = map['defines'].get('FLASH_SPI_INSTANCE')
        if cs:
            # TODO: add more drivers
            suppored_flash_chips = [
                'M25P16',
                'W25M',
                'W25M02G',
                'W25M512',
                'W25N01G',
                'W25N02K',
            ]
            file.write("#define USE_FLASHFS\n")
            file.write("#define ENABLE_BLACKBOX_LOGGING_ON_SPIFLASH_BY_DEFAULT\n")
            for flash in suppored_flash_chips:
                file.write("#define USE_FLASH_%s\n" % (flash))
                file.write("#define %s_SPI_BUS BUS_%s\n" % (flash, spiflash_bus))
                file.write("#define %s_CS_PIN %s\n" % (flash, cs))

    # SD Card:
    use_sdcard = False
    for i in range(1, 9):
        sdio_cmd = findPinByFunction("SDIO_CMD_%i" % (i), map)

        if sdio_cmd:
            if not use_sdcard:
                file.write("#define USE_SDCARD\n")
                file.write("#define USE_SDCARD_SDIO\n")
                file.write("#define ENABLE_BLACKBOX_LOGGING_ON_SDCARD_BY_DEFAULT\n")
                use_sdcard = True
            file.write("#define SDCARD_SDIO_4BIT\n")
            file.write("#define SDCARD_SDIO_DEVICE SDIODEV_%i\n" % (i))
    
    # PINIO
    use_pinio = False
    for i in range(1, 9):
        pinio = findPinByFunction("PINIO%i" % (i), map)
        if pinio != None:
            if not use_pinio:
                use_pinio = True
                file.write("\n// PINIO\n\n")
                file.write("#define USE_PINIO\n")
                file.write("#define USE_PINIOBOX\n")
            file.write("#define PINIO%i_PIN %s\n" % (i, pinio))

    file.write("\n\n// Others\n\n")

    pwm_outputs = getPwmOutputCount(map)
    file.write("#define MAX_PWM_OUTPUT_PORTS %i\n" % (pwm_outputs))
    file.write("#define USE_SERIAL_4WAY_BLHELI_INTERFACE\n")

    file.write("#define USE_DSHOT\n")
    file.write("#define USE_ESC_SENSOR\n")

    if 'DEFAULT_VOLTAGE_METER_SCALE' in map['defines']:
        file.write("#define VOLTAGE_METER_SCALE %s\n" % (map['defines']['DEFAULT_VOLTAGE_METER_SCALE']))
    if 'DEFAULT_CURRENT_METER_SCALE' in map['defines']:
        file.write("#define CURRENT_METER_SCALE %s\n" % (map['defines']['DEFAULT_CURRENT_METER_SCALE']))


    port_config = getPortConfig(map)

    file.write(port_config)

    file.close()
    return


def mcu2timerKey(mcu):
    m = re.search(r'^AT32F435[GM]', mcu)
    if m:
        return 'AT32F435'
    
    m = re.search(r'^STM32F405', mcu)
    if m:
        return 'STM32F405'

    m = re.search(r'^STM32F7[2Xx]2', mcu)
    if m:
        return 'STM32F722'

    m = re.search(r'^STM32F7[Xx46]5', mcu)
    if m:
        return 'STM32F745'

    m = re.search(r'^STM32H7[45]3', mcu)
    if m:
        return 'STM32H743'

    print ("Unsupported MCU: %s" % (mcu))
    sys.exit(-1)


def getTimerInfo(map, pin):
    with open("timer_pins.yaml", "r") as f:
        pindb = yaml.safe_load(f)
        f.close()

        mcu = map['mcu']['type']
        tk = mcu2timerKey(mcu)

        if not tk in pindb:
            print ("PINDB not available for MCU: %s" % (mcu))
            sys.exit(-1)

        timers = pindb[tk].get(pin, None)

        if timers:
            result = []
            for ti in timers:
                timer = list(ti.keys())[0]
                channel = ti[timer]

                result.append([timer, channel])
            
            return result

    return None

def writeTargetC(folder, map):
    file = open(folder + '/target.c', "w+")

    file.write("""/*
 * This file is part of INAV.
 *
 * INAV is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * INAV is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with INAV.  If not, see <http://www.gnu.org/licenses/>.
 *
 * This target has been autgenerated by bf2inav.py
 */

#include <stdint.h>

#include "platform.h"

#include "drivers/bus.h"
#include "drivers/io.h"
#include "drivers/pwm_mapping.h"
#include "drivers/timer.h"
#include "drivers/pinio.h"
//#include "drivers/sensor.h"

""")

    #for supportedgyro in ['BMI160', 'BMI270', 'ICM20689', 'ICM42605', 'MPU6000', 'MPU6500', 'MPU9250']:
    #    found = False
    #    for var in ['USE_ACCGYRO_', 'USE_ACC_', 'USE_ACC_SPI', 'USE_GYRO_', 'USE_GYRO_SPI_']:
    #            val = var + supportedgyro
    #            if val in map['empty_defines']:
    #                found = True
    #                break
        
    #    if found:
    #        file.write("//BUSDEV_REGISTER_SPI_TAG(busdev_%s,  DEVHW_%s,  %s_SPI_BUS,   %s_CS_PIN,   NONE,   0,  DEVFLAGS_NONE,  IMU_%s_ALIGN);\n" % (supportedgyro.lower(), supportedgyro, supportedgyro, supportedgyro, supportedgyro))

    snum=1
    file.write("\ntimerHardware_t timerHardware[] = {\n")

    motors = findPinsByFunction("MOTOR", map)
    if motors:
        for motor in motors:
            timerInfo = getTimerInfo(map, motor)
            if timerInfo:
                first = True
                #print (timerInfo)
                for (t, ch) in timerInfo:
                    if first:
                        file.write("    DEF_TIM(%s, %s, %s, TIM_USE_OUTPUT_AUTO, 0, %s), // S%i\n" % (t, ch, motor, 0, snum))
                        first = False
                        snum += 1
                    else:
                        file.write("    //DEF_TIM(%s, %s, %s, TIM_USE_OUTPUT_AUTO, 0, %s),\n" % (t, ch, motor, 0))
                file.write("\n")

    servos = findPinsByFunction("SERVO", map)
    if servos:
        for servo in servos:
            timerInfo = getTimerInfo(map, servo)
            if timerInfo:
                first = True
                #print (timerInfo)
                for (t, ch) in timerInfo:
                    if first:
                        file.write("    DEF_TIM(%s, %s, %s, TIM_USE_OUTPUT_AUTO, 0, %s), // S%i\n" % (t, ch, servo, 0, snum))
                        first = False
                        snum += 1
                    else:
                        file.write("    //DEF_TIM(%s, %s, %s, TIM_USE_OUTPUT_AUTO, 0, %s),\n" % (t, ch, servo, 0))
                file.write("\n")

    beeper = findPinByFunction("BEEPER", map)
    if beeper:
        timerInfo = getTimerInfo(map, beeper)
        if timerInfo:
            first = True
            #print ("BEEPER: %s" % (timerInfo))
            for (t, ch) in timerInfo:
                if first:
                    file.write("    DEF_TIM(%s, %s, %s, TIM_USE_BEEPER, 0, %s),\n" % (t, ch, beeper, 0))
                    first = False
                else:
                    file.write("    //DEF_TIM(%s, %s, %s, TIM_USE_BEEPER, 0, %s),\n" % (t, ch, beeper, 0))
            file.write("\n")

    led = findPinByFunction("LED_STRIP", map)
    if led:
        timerInfo = getTimerInfo(map, led)
        if timerInfo:
            first = True
            #print (timerInfo)
            for (t, ch) in timerInfo:
                if first:
                    file.write("    DEF_TIM(%s, %s, %s, TIM_USE_LED, 0, %s),\n" % (t, ch, led, 0))
                    first = False
                else:
                    file.write("    //DEF_TIM(%s, %s, %s, TIM_USE_LED, 0, %s),\n" % (t, ch, led, 0))
            file.write("\n")

    file.write("""};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);
""")

    file.close()
    return

def writeConfigC(folder, map):
    file = open(folder + '/config.c', "w+")

    file.write("""/*
 * This file is part of INAV.
 *
 * INAV is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * INAV is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with INAV.  If not, see <http://www.gnu.org/licenses/>.
 *
 * This target has been autgenerated by bf2inav.py
 */

#include <stdint.h>

#include "platform.h"

#include "fc/fc_msp_box.h"
#include "fc/config.h"

#include "io/piniobox.h"

void targetConfiguration(void)
{
""")
    #//pinioBoxConfigMutable()->permanentId[0] = BOX_PERMANENT_ID_USER1;
    #//pinioBoxConfigMutable()->permanentId[1] = BOX_PERMANENT_ID_USER2;
    #//beeperConfigMutable()->pwmMode = true;
    file.write("""
}

""")

    file.close()
    return

def writeTarget(outputFolder, map):
    writeCmakeLists(outputFolder, map)
    writeTargetH(outputFolder, map)
    writeTargetC(outputFolder, map)
    writeConfigC(outputFolder, map)

    return

def buildMap(inputFile):
    map = { 'defines': {}, 'empty_defines': [], 'features': ['FEATURE_OSD', 'FEATURE_TELEMETRY', 'FEATURE_CURRENT_METER', 'FEATURE_VBAT', 'FEATURE_TX_PROF_SEL', 'FEATURE_BLACKBOX'], 'pins': {}, 'funcs': {}, 'timer_pin_map': {}}

    f = open(inputFile, 'r')
    while True:
        l = f.readline()
        if not l:
            break
        m = re.search(r'^#define\s+FC_TARGET_MCU\s+([0-9A-Za-z]+)$', l)
        if m:
            map['mcu'] = {'type': m.group(1)}

        m = re.search(r'^#define\s+BOARD_NAME\s+(\w+)$', l)
        if m:
            map['board_name'] = m.group(1)

        m = re.search(r'^#define\s+MANUFACTURER_ID\s+(\w+)$', l)
        if m:
            map['manufacturer_id'] = m.group(1)

        m = re.search(r'^#define\s+(\w+)\s*$', l)
        if m:
            map['empty_defines'].append(re.sub('ICM42688P', 'ICM42605', m.group(1)))
        
        m = re.search(r'^\s*#define\s+DEFAULT_FEATURES\s+\((.+?)\)\s*$', l)
        if m:
            features = m.group(1).split('|')
            for feat in features:
                feat = feat.strip()
                if not feat in map['features']:
                    map['features'].append(feat)

        m = re.search(r'^#define\s+(\w+)\s+(\S+)\s*$', l)
        if m:
            map['defines'][m.group(1)] = m.group(2)



        # i: timer index
        # p: pin
        # o: timer channel? 1 = first
        # d: dma opts
        #                i  p     o   d
        # TIMER_PIN_MAP( 0, PB8 , 2, -1) \
        m = re.search(r'^\s*TIMER_PIN_MAP\s*\(\s*(\d+)\s*,\s*([0-9A-Za-z]+)\s*,\s*(\d+)\s*,\s*(-?\d+)\s*\).+', l) 
        if m:
            map['timer_pin_map'][m.group(1)] = {
                'i': m.group(1),
                'p': m.group(2),
                'o': m.group(3),
                'd': m.group(4)
            }

        
        m = re.search(r'^\s*#define\s+(\w+)_PIN\s+([A-Z0-9]+)\s*$', l)
        if m:
            pin = m.group(2)
            func = m.group(1)
            if not map['funcs'].get(func):
                map['funcs'][func] = {}

            map['funcs'][func] = pin

            if not map['pins'].get(pin):
                map['pins'][pin] = {}

            map['pins'][pin] = func


        #m = re.search(r'^feature\s+(-?\w+)$', l)
        #if m:
        #    map['features'].append(m.group(1))

        #m = re.search(r'^resource\s+(-?\w+)\s+(\d+)\s+(\w+)$', l)
        #if m:
        #    resource_type = m.group(1)
        #    resource_index = m.group(2)
        #    pin = translatePin(m.group(3))
        #    if not map['pins'].get(pin):
        #        map['pins'][pin] = {}

        #    map['pins'][pin]['function'] = translateFunctionName(resource_type, resource_index)

        #m = re.search(r'^timer\s+(\w+)\s+AF(\d+)$', l)
        #if m:
        #    pin = translatePin(m.group(1))
        #    if not map['pins'].get(pin):
        #        map['pins'][pin] = {}
        #    
        #    map['pins'][pin]['AF'] = m.group(2)

        #m = re.search(r'^#\s*pin\s+(\w+):\s*(TIM\d+)\s+(CH\d+).+$', l)
        #if m:
        #    pin = translatePin(m.group(1))
        #    if not map['pins'].get(pin):
        #        map['pins'][pin] = {}
            
        #    map['pins'][pin]['TIM'] = m.group(2)
        #    map['pins'][pin]['CH'] = m.group(3)
        
        #m = re.search(r'^dma\s+([A-Za-z0-9]+)\s+([A-Za-z0-9]+)\s+(\d+).*$', l)
        #if m:
        #    if(m.group(1) == 'ADC'):
        #        pin = 'ADC' + m.group(2)
        #    else:
        #        pin = translatePin(m.group(2))
        #    if not map['dmas'].get(pin):
        #        map['dmas'][pin] = {}
        #    map['dmas'][pin]['DMA'] = m.group(3)

#      1     2         3         4
# pin B04: DMA1 Stream 4 Channel 5
    
        #m = re.search(r'^#\s+pin\s+(\w+):\s+(DMA\d+)\s+Stream\s+(\d+)\s+Channel\s+(\d+)\s*$', l)
        #if m:
        #    pin = translatePin(m.group(1))
        #    if not map['pins'].get(pin):
        #        map['pins'][pin] = {}
            
        #    map['pins'][pin]['DMA_STREAM'] = m.group(3)
        #    map['pins'][pin]['DMA_CHANNEL'] = m.group(4)
        
        #m = re.search(r'^#\s+ADC\s+(\d+):\s+(DMA\d+)\s+Stream\s+(\d+)\s+Channel\s+(\d+)\s*$', l)
        #if m:
        #    pin = 'ADC' + m.group(1)
        #    if not map['dmas'].get(pin):
        #        map['dmas'][pin] = {}
            
        #    map['dmas'][pin]['DMA_STREAM'] = m.group(3)
        #    map['dmas'][pin]['DMA_CHANNEL'] = m.group(4)

        #m = re.search(r'^#\s+TIMUP\s+(\d+):\s+(DMA\d+)\s+Stream\s+(\d+)\s+Channel\s+(\d+)\s*$', l)
        #if m:
        #    pin = 'TIMUP' + m.group(1)
        #    if not map['dmas'].get(pin):
        #        map['dmas'][pin] = {}
        #    map['dmas'][pin]['DMA_STREAM'] = m.group(3)
        #    map['dmas'][pin]['DMA_CHANNEL'] = m.group(4)

        #m = re.search(r'^#\s+ADC\s+(\d+):\s+(DMA\d+)\s+Stream\s+(\d+)\s+Request\s+(\d+)\s*$', l)
        #if m:
        #    pin = 'ADC' + m.group(1)
        #    if not map['dmas'].get(pin):
        #        map['dmas'][pin] = {}
            
        #    map['dmas'][pin]['DMA_STREAM'] = m.group(3)
        #    map['dmas'][pin]['DMA_REQUEST'] = m.group(4)

        #m = re.search(r'^#\s+TIMUP\s+(\d+):\s+(DMA\d+)\s+Stream\s+(\d+)\s+Channel\s+(\d+)\s*$', l)
        #if m:
        #    pin = 'TIMUP' + m.group(1)
        #    if not map['dmas'].get(pin):
        #        map['dmas'][pin] = {}
            
        #    map['dmas'][pin]['DMA_STREAM'] = m.group(3)
        #    map['dmas'][pin]['DMA_CHANNEL'] = m.group(4)

        #m = re.search(r'^#\s+TIMUP\s+(\d+):\s+(DMA\d+)\s+Stream\s+(\d+)\s+Request\s+(\d+)\s*$', l)
        #if m:
        #    pin = 'TIMUP' + m.group(1)
        #    if not map['dmas'].get(pin):
        #        map['dmas'][pin] = {}
        #    map['dmas'][pin]['DMA_STREAM'] = m.group(3)
        #    map['dmas'][pin]['DMA_REQUEST'] = m.group(4)
 
        #m = re.search(r'^serial\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)$', l)
        #if m:
        #    idx = m.group(1)
        #    if not map['serial'].get(idx):
        #        map['serial'][idx] = {}
            
        #    map['serial'][idx]['FUNCTION'] = m.group(2)
        #    map['serial'][idx]['MSP_BAUD'] = m.group(3)
        #    map['serial'][idx]['GPS_BAUD'] = m.group(4)
        #    map['serial'][idx]['TELEMETRY_BAUD'] = m.group(5)
        #    map['serial'][idx]['BLACKBOX_BAUD'] = m.group(6)

        #m = re.search(r'^set\s+(\w+)\s*=\s*(\w+)$', l)
        #if m:
        #    map['variables'][m.group(1)] = m.group(2)
 

    return map

def printHelp():
    print ("%s -i bf-target.config -o output-directory" % (sys.argv[0]))
    print ("    -i | --input-config=<file>     -- print this help")
    print ("    -o | --output-dir=<targetdir>  -- print this help")
    print ("    -h | --help                    -- print this help")
    print ("    -v | --version                 -- print version")
    return

def main(argv):
    inputfile = ''
    outputdir = '.'
    global version

    try:
        opts, args = getopt.getopt(argv,"hvi:o:", ["input-config=", "output-dir=", 'version', 'help'])
    except getopt.GeoptError:
        printHelp()
        sys.exit(2)

    for opt, arg in opts:
        if opt in ('-h', '--help'):
            printHelp()
            sys.exit(1)
        elif opt in ('-i', '--input-config'):
            inputfile = arg
        elif opt in ('-o', '--output-dir'):
            outputdir = arg
        elif opt in ('-v', '--version'):
            print ("%s: %s" % (sys.argv[0], version))
            sys.exit(0)

    if (not os.path.isfile(inputfile) ):
      print("no such file %s" % inputfile)
      sys.exit(2)
    if (not os.path.isdir(outputdir) ):
      print("no such directory %s" % outputdir)
      sys.exit(2)
    else:
      targetDefinition = buildMap(inputfile)


    map = buildMap(inputfile)

    print (json.dumps(map, indent=4))

    writeTarget(outputdir, map)

    sys.exit(0)


if( __name__ == "__main__"):
    main(sys.argv[1:])


