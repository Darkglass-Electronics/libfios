#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2024 Filipe Coelho <falktx@darkglass.com>
# SPDX-License-Identifier: ISC

import os
import sys

from ctypes import (
    Structure,
    POINTER,
    cdll,
    c_bool,
    c_char_p,
    c_float,
    pointer,
)

if sys.platform == 'darwin':
    ext = 'dylib'
elif sys.platform == 'win32':
    ext = 'dll'
else:
    ext = 'so'

tryPath1 = os.path.join(os.path.join(os.path.dirname(__file__), 'libfios.' + ext))
tryPath2 = os.path.join(os.path.join(os.path.dirname(__file__), '..', 'libfios.' + ext))

if os.path.exists(tryPath1):
    libfios = cdll.LoadLibrary(tryPath1)
elif os.path.exists(tryPath2):
    libfios = cdll.LoadLibrary(tryPath2)
else:
    libfios = cdll.LoadLibrary('libfios.' + ext)

# use a well known size for commands, giving enough space for a small single argument
CMD_SIZE = 2 + 10 + 1

# maximum size allowed in file APIs
MAX_FILE_SIZE = 0x7fffffff

# maximum payload size, used to receive data after a command
MAX_PAYLOAD_SIZE = 0x2000

# opaque API structures
class fios_serial_t(Structure):
    pass

class fios_file_t(Structure):
    pass

# ---------------------------------------------------------------------------------------------------------------------
# serial IO

# Open the serial port at @a devpath
libfios.fios_serial_open.argtypes = (c_char_p,)
libfios.fios_serial_open.restype  = POINTER(fios_serial_t)

def fios_serial_open(devpath):
    return libfios.fios_serial_open(devpath.encode("utf-8"))

# Close a serial port
libfios.fios_serial_close.argtypes = (POINTER(fios_serial_t),)
libfios.fios_serial_close.restype  = None

def fios_serial_close(s):
    libfios.fios_serial_close(s)

# ---------------------------------------------------------------------------------------------------------------------
# serial communication

# ---------------------------------------------------------------------------------------------------------------------
# file operations (using background threads)

# fios_file_status_t
fios_file_status_error = 0
fios_file_status_in_progress = 1
fios_file_status_completed = 2

# prepare to receive data from a serial port into the file @a outpath
# a background thread is used for receiving data from the serial port and writing to the file
# use @fios_file_idle to query current progress and @fios_file_close when done
libfios.fios_file_receive.argtypes = (POINTER(fios_serial_t), c_char_p,)
libfios.fios_file_receive.restype  = POINTER(fios_file_t)

def fios_file_receive(s, outpath):
    return libfios.fios_file_receive(s, outpath.encode("utf-8"))

# prepare to send data from the file @a inpath into a serial port
# a background thread is used for reading the file and sending data to the serial port
# use `fios_file_idle` to query current progress and `fios_file_close` when done
libfios.fios_file_send.argtypes = (POINTER(fios_serial_t), c_char_p,)
libfios.fios_file_send.restype  = POINTER(fios_file_t)

def fios_file_send(s, inpath):
    return libfios.fios_file_send(s, inpath.encode("utf-8"))

# check status of an active serial file transfer
# NOTE in python this returns (status, progress) where:
# - `status` is normal return value
# - `progress` is current progress between 0.0 and 1.0
libfios.fios_file_idle.argtypes = (POINTER(fios_file_t), POINTER(c_float),)
libfios.fios_file_idle.restype  = c_int

def fios_file_idle(f):
    progress = c_float(0.0)
    return (libfios.fios_file_idle(f, pointer(progress)), progress.value)

# get the current progress of an active serial file transfer
# returns a value between 0.0 and 1.0
libfios.fios_file_get_progress.argtypes = (POINTER(fios_file_t),)
libfios.fios_file_get_progress.restype  = c_float

def fios_file_get_progress(f):
    return libfios.fios_file_get_progress(f)

# close the file operation
# must still be called even if `fios_file_idle` returns false
libfios.fios_file_close.argtypes = (POINTER(fios_file_t),)
libfios.fios_file_close.restype  = None

def fios_file_close(f):
    libfios.fios_file_close(f)

# get the error message for the case where `fios_file_idle` returns `fios_file_status_error`
# must not be called after `fios_file_close`
libfios.fios_file_get_last_error.argtypes = (POINTER(fios_file_t),)
libfios.fios_file_get_last_error.restype  = c_char_p

def fios_file_get_last_error(f):
    return libfios.fios_file_get_last_error(f).decode("utf-8")

# ---------------------------------------------------------------------------------------------------------------------
