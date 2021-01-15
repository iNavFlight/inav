#!/usr/bin/python3
# -*- coding: utf-8 -*-
__author__ = 'Fabien Poussin'
__version__ = '0.1'

import os
import re
from argparse import ArgumentParser
from traceback import print_exc
from shutil import copy

parser = ArgumentParser(description='Generate ChibiOS-Contrib config and make files from ChibiOS')
parser.add_argument('-s', '--src', default='../../ChibiOS-RT', type=str, help="ChibiOS folder")
parser.add_argument('-d', '--dst', default='..', type=str, help='ChibiOS-Contrib folder')


def makefile(lines):

    for l in range(len(lines)):

        if 'CHIBIOS =' in lines[l]:
            lines[l] = lines[l][:-1] + '/../ChibiOS-RT\n'
            lines.insert(l + 1, 'CHIBIOS_CONTRIB = $(CHIBIOS)/../ChibiOS-Contrib\n')

        if '$(CHIBIOS)/os/hal/hal.mk' in lines[l] \
                or '$(CHIBIOS)/os/hal/ports/' in lines[l] \
                or '$(CHIBIOS)/os/various' in lines[l]            :
            lines[l] = lines[l].replace('CHIBIOS', 'CHIBIOS_CONTRIB')

    return "".join(lines)


def halconf(lines):

    idx_end = lines.index('#endif /* HALCONF_H */\n')
    lines.insert(idx_end - 1, '\n')
    lines.insert(idx_end - 1, '#include "halconf_community.h"')
    lines.insert(idx_end - 1, '\n')

    return "".join(lines)


def mcuconf(lines):

    idx_end = lines.index('#endif /* MCUCONF_H */\n')
    lines.insert(idx_end - 1, '\n')
    lines.insert(idx_end - 1, '#include "mcuconf_community.h"')
    lines.insert(idx_end - 1, '\n')

    return "".join(lines)


if __name__ == '__main__':

    args = parser.parse_args()
    sources = {}

    for folder in ['testhal']:

        for family in os.scandir(args.src + '/{}/STM32/'.format(folder)):
            if not family.name[0].isupper() or not family.is_dir():
                continue

            for test in os.scandir(family.path):
                try:
                    sources[family.name] = {'makefile': None, 'halconf': None, 'mcuconf': None}

                    with open(test.path + '/Makefile', 'r') as file:
                        sources[family.name]['makefile'] = makefile(file.readlines())

                    with open(test.path + '/halconf.h', 'r') as file:
                        sources[family.name]['halconf'] = halconf(file.readlines())

                    with open(test.path + '/mcuconf.h', 'r') as file:
                        sources[family.name]['mcuconf'] = mcuconf(file.readlines())

                except Exception as e:
                    print(test.path, e)
                    del sources[family.name]
                    continue

                break

        for family in os.scandir(args.dst + '/{}/STM32/'.format(folder)):
            if not family.name[0].isupper() or not family.is_dir():
                continue

            for test in os.scandir(family.path):
                copy('templates/halconf_community.h', test.path)
                copy('templates/mcuconf_community.h', test.path)

                try:
                    with open(test.path + '/Makefile', 'w') as file:
                        file.write(sources[family.name]['makefile'])

                    with open(test.path + '/halconf.h', 'w') as file:
                        file.write(sources[family.name]['halconf'])

                    with open(test.path + '/mcuconf.h', 'w') as file:
                        file.write(sources[family.name]['mcuconf'])

                    print('updated', test.path)
                except KeyError as e:
                    print('Missing family data', e)
