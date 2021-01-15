#!/usr/bin/python
# -*- coding: utf-8 -*-
__author__ = 'Fabien Poussin'
__version__ = '0.3'


from xml.etree import ElementTree as etree
from jinja2 import Template
from os.path import expanduser, sep, dirname, abspath
from argparse import ArgumentParser
from traceback import print_exc
import re
import pprint


pretty_print = pprint.PrettyPrinter(indent=2)


def pprint(*kwargs):
    pretty_print.pprint(kwargs)

PIN_MODE_INPUT = "PIN_MODE_INPUT({0})"
PIN_MODE_OUTPUT = "PIN_MODE_OUTPUT({0})"
PIN_MODE_ALTERNATE = "PIN_MODE_ALTERNATE({0})"
PIN_MODE_ANALOG = "PIN_MODE_ANALOG({0})"
PIN_ODR_LOW = "PIN_ODR_LOW({0})"
PIN_ODR_HIGH = "PIN_ODR_HIGH({0})"
PIN_OTYPE_PUSHPULL = "PIN_OTYPE_PUSHPULL({0})"
PIN_OTYPE_OPENDRAIN = "PIN_OTYPE_OPENDRAIN({0})"
PIN_OSPEED_VERYLOW = "PIN_OSPEED_VERYLOW({0})"
PIN_OSPEED_LOW = "PIN_OSPEED_LOW({0})"
PIN_OSPEED_MEDIUM = "PIN_OSPEED_MEDIUM({0})"
PIN_OSPEED_HIGH = "PIN_OSPEED_HIGH({0})"
PIN_PUPDR_FLOATING = "PIN_PUPDR_FLOATING({0})"
PIN_PUPDR_PULLUP = "PIN_PUPDR_PULLUP({0})"
PIN_PUPDR_PULLDOWN = "PIN_PUPDR_PULLDOWN({0})"
PIN_AFIO_AF = "PIN_AFIO_AF({0}, {1})"

FMT = '{0}'
FMT_DEF = '({0})'

PIN_CONF_LIST = ['MODER', 'OTYPER', 'OSPEEDR', 'PUPDR', 'ODR']
PIN_CONF_LIST_AF = ['AFRL', 'AFRH']

DEFAULT_PAD = {"SIGNAL": "UNUSED",
               "LABEL": "",
               "MODER": PIN_MODE_ANALOG,
               "OTYPER": PIN_OTYPE_PUSHPULL,
               "OSPEEDR": PIN_OSPEED_VERYLOW,
               "PUPDR": PIN_PUPDR_FLOATING,
               "ODR": PIN_ODR_LOW}

PIN_MODE_TRANSLATE = {"GPIO_MODE_AF_PP": PIN_MODE_ALTERNATE,
                      "GPIO_MODE_ANALOG": PIN_MODE_ANALOG,
                      "GPIO_MODE_INPUT": PIN_MODE_INPUT,
                      "GPIO_MODE_OUTPUT": PIN_MODE_OUTPUT,
                      "GPIO_MODE_OUTPUT_PP": PIN_MODE_OUTPUT,
                      "GPIO_MODE_OUTPUT_OD": PIN_MODE_OUTPUT}

PIN_OTYPE_TRANSLATE = {"GPIO_MODE_OUTPUT_PP": PIN_OTYPE_PUSHPULL,
                        "GPIO_MODE_OUTPUT_OD": PIN_OTYPE_OPENDRAIN}

PIN_OSPEED_TRANSLATE = {"GPIO_SPEED_FREQ_LOW": PIN_OSPEED_VERYLOW,
                        "GPIO_SPEED_FREQ_MEDIUM": PIN_OSPEED_LOW,
                        "GPIO_SPEED_FREQ_HIGH": PIN_OSPEED_MEDIUM,
                        "GPIO_SPEED_FREQ_VERY_HIGH": PIN_OSPEED_HIGH
                        }

PIN_PUPDR_TRANSLATE = {"GPIO_NOPULL": PIN_PUPDR_FLOATING,
                       "GPIO_PULLUP": PIN_PUPDR_PULLUP,
                       "GPIO_PULLDOWN": PIN_PUPDR_PULLDOWN}

PIN_ODR_TRANSLATE = {"GPIO_PIN_SET": PIN_ODR_HIGH,
                     "GPIO_PIN_CLEAR": PIN_ODR_LOW,
                     "GPIO_PIN_RESET": PIN_ODR_LOW }

parser = ArgumentParser(description='Generate ChibiOS GPIO header file from STM32CubeMX project files.')
group = parser.add_mutually_exclusive_group(required=False)
group.add_argument('-m', '--mx', default='', type=str, help='STM32CubeMX path. Invalid if -g is used.')
group.add_argument('-g', '--gpio', default='', type=str, help='STM32CubeMX Gpio file, if you don\'t have STM32CubeMX installed. Invalid if -m is used.')
parser.add_argument('-p', '--project', required=True, type=str, help="STM32CubeMX Project file")
parser.add_argument('-o', '--output', default='board_gpio.h', type=str, help='Output file name')


def open_xml(filename):
    #  Remove namespace
    with open(filename, 'r') as xmlfile:
        xml = re.sub(' xmlns="[^"]+"', '', xmlfile.read(), count=1)
    return etree.fromstring(xml)


def char_range(c1, c2):
    """Generates the characters from `c1` to `c2`, inclusive."""
    for c in range(ord(c1), ord(c2)+1):
        yield chr(c)


def get_gpio_file(proj_file, mx_path):

    mcu_name = None
    gpio_file = None
    path = None
    mcu_info = None

    print('Opening ' + proj_file)
    with open(proj_file, 'r') as f:
        proj_data = f.readlines()

    for l in proj_data:
        if l.startswith('Mcu.Name'):
            print('MCU is ' + l.split('=')[-1].strip())
            mcu_name = '{}.xml'.format(l.split('=')[-1].strip())

    if not mcu_name:
        print('Could not find MCU name in project file')
        exit(1)

    if "STM32F1" in mcu_name:
        print('STM32F1xx are not compatible with this script. (old GPIO)')
        exit(1)

    found = False
    for p in (mx_path,
              '{0}{1}STM32CubeMX'.format(expanduser("~"), sep),
              'C:{0}Program Files{0}STMicroelectronics{0}STM32Cube{0}STM32CubeMX'.format(sep)):

        if not p:
            continue
        try:
            path = '{1}{0}db{0}mcu{0}'.format(sep, p)
            mcu_info = open_xml(path + mcu_name)
            found = True
            break
        except IOError:
            continue

    if not found:
        print('Could not find GPIO file')
        exit(1)

    print('Opened ' + path)

    for ip in mcu_info.findall("IP"):
        if ip.attrib['Name'] == 'GPIO':
            gpio_file = '{0}{2}IP{2}GPIO-{1}_Modes.xml'.format(path,
                                                               ip.attrib['Version'],
                                                               sep)

    return gpio_file


def read_gpio(filename):
    gpio = {'ports': {}, 'defaults': {}, 'modes': {}}

    print('Opening GPIO file ' + filename)
    root = open_xml(filename)

    gpio['defaults']['GPIO_Mode'] = 'GPIO_MODE_ANALOG'

    for modes in root.findall("RefParameter"):
        try:
            name = modes.attrib['Name']
            gpio['defaults'][name] = modes.attrib['DefaultValue']
            gpio['modes'][name] = []
        except KeyError as e:
            continue

        if 'GPIO_' not in name:
            continue

        for m in modes.findall("PossibleValue"):
            prop_val = m.attrib['Value']
            gpio['modes'][name].append(prop_val)

    for pin in root.findall('GPIO_Pin'):
        try:
            port = pin.attrib['Name'][1]
            num = pin.attrib['Name'][2:]
            # remove notes from pin name (e.g. PH0 - OSC_IN)
            num = num.split('-')[0].strip()
            num = int(num)

            if port not in gpio['ports']:
                gpio['ports'][port] = {}
            if num not in gpio['ports'][port]:
                gpio['ports'][port][num] = {}
        except ValueError as e:
            continue

        for s in pin.findall('PinSignal'):
            try:
                af = s.find('SpecificParameter/PossibleValue').text
                af = int(''.join(af.split('_')[1])[2:])
                gpio['ports'][port][num][s.attrib['Name']] = af
            except ValueError as e:
                print_exc(e)
            except AttributeError as e:
                print_exc(e)

    return gpio


# Extract signals from IOC
def read_project(gpio, filename):

    print('Opening project file ' + filename)
    with open(filename, 'r') as mx_file:
        tmp = mx_file.readlines()
    pads = {}

    # Default all pads to analog
    for p in gpio['ports'].keys():
        pads[p] = {}
        for i in range(0, 16):
            pads[p][i] = DEFAULT_PAD.copy()
            pads[p][i]['PUPDR'] = PIN_PUPDR_TRANSLATE[gpio['defaults']['GPIO_PuPdOD']]
            pads[p][i]['OTYPER'] = PIN_OTYPE_TRANSLATE[gpio['defaults']['GPIO_ModeDefaultOutputPP']]
            pads[p][i]['OSPEEDR'] = PIN_OSPEED_TRANSLATE[gpio['defaults']['GPIO_Speed']]

    for t in tmp:
        if re.search(r"^P[A-Z]\d{1,2}(-OSC.+)?\.", t, re.M):
            split = t.split('=')
            pad_name = split[0].split(".")[0]
            pad_port = pad_name[1:2]
            pad_num = int(pad_name[2:4].replace('.', '').replace('-', ''))
            pad_prop = split[0].split(".")[-1]
            prop_value = split[-1].rstrip('\r\n')


            if pad_prop == "Signal":
                if 'S_TIM' in prop_value:
                    prop_value = prop_value[2:]

                if prop_value.startswith('ADC') \
                        or 'DAC' in prop_value \
                        or 'OSC' in prop_value:
                    pads[pad_port][pad_num]["MODER"] = PIN_MODE_ANALOG
                    pads[pad_port][pad_num]["SIGNAL"] = prop_value
                elif 'GPIO_Output' == prop_value:
                    pads[pad_port][pad_num]["MODER"] = PIN_MODE_OUTPUT
                elif 'GPIO_Input' == prop_value:
                    pads[pad_port][pad_num]["MODER"] = PIN_MODE_INPUT
                else:
                    # workaround for different names in project and gpio defs
                    if "FSMC" in prop_value:
                        prop_value = re.sub(r"FSMC_D([0-9]+)_DA[0-9]+",
                                r"FSMC_D\1", prop_value)

                    pads[pad_port][pad_num]["SIGNAL"] = prop_value
                    pads[pad_port][pad_num]["MODER"] = PIN_MODE_ALTERNATE
                    pads[pad_port][pad_num]["OSPEEDR"] = PIN_OSPEED_MEDIUM
            elif pad_prop == "GPIO_Mode":
                pads[pad_port][pad_num]["MODER"] = PIN_MODE_TRANSLATE[prop_value]
            elif pad_prop == "GPIO_Label":
                pads[pad_port][pad_num]["LABEL"] = prop_value
            elif pad_prop == "GPIO_PuPd":
                pads[pad_port][pad_num]["PUPDR"] = PIN_PUPDR_TRANSLATE[prop_value]
            elif pad_prop == "GPIO_ModeDefaultOutputPP":
                pads[pad_port][pad_num]["OTYPER"] = PIN_OTYPE_TRANSLATE[prop_value]
                pads[pad_port][pad_num]["MODER"] = PIN_MODE_OUTPUT
            elif pad_prop == "GPIO_Speed":
                pads[pad_port][pad_num]["OSPEEDR"] = PIN_OSPEED_TRANSLATE[prop_value]
            elif pad_prop == "PinState":
                pads[pad_port][pad_num]["ODR"] = PIN_ODR_TRANSLATE[prop_value]
            elif pad_prop == "Mode":
                if "I2C" in prop_value:
                    pads[pad_port][pad_num]["OTYPER"] = PIN_OTYPE_OPENDRAIN


    return pads


# Add defines for all pins with labels
def gen_defines(project):
    defines = {}

    for port_key in sorted(project.keys()):
        for pad_key in sorted(project[port_key].keys()):

            pad_data = project[port_key][pad_key]
            if pad_data['SIGNAL'] != 'UNUSED' and not pad_data['LABEL']:
                pad_data['LABEL'] = pad_data['SIGNAL']
            pad_data['LABEL'] = pad_data['LABEL'].replace('-', '_')
            label = pad_data['LABEL']
            signal = pad_data['SIGNAL']
            if not label:
                continue

            defines['PORT_'+label] = 'GPIO' + port_key
            defines['PAD_'+label] = pad_key
            defines['LINE_'+label] = 'PAL_LINE(GPIO' + port_key
            defines['LINE_'+label] += ', ' + str(pad_key) + 'U)'

            match = re.search(r"TIM(\d+)_CH(\d)$", signal)
            if match:
                timer = match.group(1)
                ch_num = int(match.group(2))
                defines['TIM_' + label] = timer
                defines['CCR_' + label] = 'CCR' + timer[-1]
                defines['PWMD_' + label] = 'PWMD' + timer[-1]
                defines['ICUD_' + label] = 'ICUD' + timer[-1]
                defines['CHN_' + label] = ch_num - 1
                continue

            match = re.search(r"ADC(\d*)_IN(\d+)$", signal)
            if match:
                adc = match.group(1)
                if len(adc) == 0:
                    adc = 1
                defines['ADC_' + label] = adc
                defines['CHN_' + label] = match.group(2)
                continue

            match = re.search(r"USART(\d+)_[RT]X$", signal)
            if match:
                defines['USART_' + label] = match.group(1)
                continue

            match = re.search(r"COMP_DAC(\d+)_group", signal)
            if match:
                defines['DAC_' + label] = match.group(1)
                continue

            match = re.search(r"I2C(\d)_S(CL|DA)", signal)
            if match:
                defines['I2C_' + label] = match.group(1)
                continue

            match = re.search(r"SPI(\d)_(MOSI|MISO|SCK|NSS)", signal)
            if match:
                defines['SPI_' + label] = match.group(1)
                continue

            match = re.search(r"CAN(\d*)_[RT]X", signal)
            if match:
                can = match.group(1)
                if len(can) == 0:
                    can = 1
                defines['CAN_' + label] = can
                continue

    return defines


# Each Port (A.B.C...)
def gen_ports(gpio, project):
    ports = {}
    for port_key in sorted(project.keys()):

        ports[port_key] = {}
        # Each property (mode, output/input...)
        for conf in PIN_CONF_LIST:
            ports[port_key][conf] = []
            for pin in project[port_key]:
                out = project[port_key][pin][conf]
                out = out.format(pin)
                ports[port_key][conf].append(out)

        conf = PIN_CONF_LIST_AF[0]
        ports[port_key][conf] = []
        for pin in range(0, 8):
            try:
                af = project[port_key][pin]['SIGNAL']
                out = PIN_AFIO_AF.format(pin, gpio['ports'][port_key][pin][af])
            except KeyError as e:
                out = PIN_AFIO_AF.format(pin, 0)
            ports[port_key][conf].append(out)

        conf = PIN_CONF_LIST_AF[1]
        ports[port_key][conf] = []
        for pin in range(8, 16):
            try:
                af = project[port_key][pin]['SIGNAL']
                out = PIN_AFIO_AF.format(pin, gpio['ports'][port_key][pin][af])
            except KeyError:
                out = PIN_AFIO_AF.format(pin, 0)
            ports[port_key][conf].append(out)

    return ports


if __name__ == '__main__':
    args = parser.parse_args()
    cur_path = dirname(abspath(__file__))

    if args.gpio:
        gpio = read_gpio(args.gpio)

    else:
        gpio_file = get_gpio_file(args.project, args.mx)
        gpio = read_gpio(gpio_file)
    proj = read_project(gpio, args.project)
    defines = gen_defines(proj)
    ports = gen_ports(gpio, proj)

    with open(cur_path + '/templates/board_gpio.tpl', 'r') as tpl_file:
        tpl = tpl_file.read()
    template = Template(tpl)

    defines_sorted = []
    for d in sorted(defines.keys()):
        defines_sorted.append((d, defines[d]))

    ports_sorted = []
    for p in sorted(ports.keys()):
        ports_sorted.append((p, ports[p]))

    template.stream(defines=defines_sorted, ports=ports_sorted).dump(args.output)

    print('File generated at ' + args.output)
