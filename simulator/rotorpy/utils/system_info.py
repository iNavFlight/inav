print('The below information will help debug installation problems.\n')

import sys
print('Python Version:')
print(sys.version)
print()

import platform
print('Operating system:')
print(platform.platform())
print()

import matplotlib
print('matplotlib:', matplotlib.__version__)

import numpy
print('numpy:     ', numpy.__version__)

import pip
print('pip:       ', pip.__version__)

import scipy
print('scipy:     ', scipy.__version__)
