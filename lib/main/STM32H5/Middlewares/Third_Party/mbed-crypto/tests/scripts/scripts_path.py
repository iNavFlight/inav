"""Add our Python library directory to the module search path.

Usage:

    import scripts_path # pylint: disable=unused-import
"""

# Copyright The Mbed TLS Contributors
# SPDX-License-Identifier: Apache-2.0
#

import os
import sys

sys.path.append(os.path.join(os.path.dirname(__file__),
                             os.path.pardir, os.path.pardir,
                             'scripts'))
