#!/usr/bin/env python3

"""
This file is part of Cleanflight.

Cleanflight is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Cleanflight is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Cleanflight.  If not, see <http://www.gnu.org/licenses/>.
"""

import os
import operator
import glob

if __name__ == '__main__':
    path = os.path.join(os.path.dirname(__file__), '..', 
                        'main', 'target', '*', 'target.h')
    files = glob.glob(path)
    defines = {}

    for file in files:
        target = os.path.basename(os.path.dirname(file))
        with open(file, 'r') as handle:
            for line in handle:
                line = line.strip()
                if line.startswith('#define'):
                    try:
                        line = line.replace('\t', ' ')
                        define = line.split(' ')[1].strip()
                        if define not in defines:
                            defines[define] = []
                        defines[define].append(target)
                    except IndexError:
                        pass

    counts = {k: len(v) for k, v in defines.items()}

    for define, count in sorted(counts.items(), key=operator.itemgetter(1)):
        print("{}\t{}\t{}".format(define, count, ', '.join(sorted(defines[define]))))
