"""
High-level MultiWii Serial Protocol toolkit with codecs, transports, and a
multi-client server for Betaflight/INAV flight controllers.
"""

from __future__ import annotations

from importlib import metadata as importlib_metadata

from .msp_api import MSPApi
from .msp_serial import MSPSerial
from .mspcodec import InavMSP, MSPCodec

try:
    __version__ = importlib_metadata.version("inav-msp")
except importlib_metadata.PackageNotFoundError:  # pragma: no cover - fallback for editable installs
    __version__ = "0.0.0"

__all__ = [
    "MSPApi",
    "MSPSerial",
    "InavMSP",
    "__version__",
]
