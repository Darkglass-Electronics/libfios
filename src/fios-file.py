#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2024 Filipe Coelho <falktx@darkglass.com>
# SPDX-License-Identifier: AGPL-3.0-or-later

import sys
import time

from libfios import (
    fios_serial_open,
    fios_serial_close,
    fios_file_send,
    fios_file_receive,
    fios_file_idle,
    fios_file_close,
    fios_file_status_in_progress,
)

def usage():
    print("Usage: %s [r|s] [device-path|auto] [file-path]", sys.argv[0])
    return 1

def main():
    if len(sys.argv) <= 3:
        return usage()

    if sys.argv[1] == "r":
        sending = False
    elif sys.argv[1] == "s":
        sending = True
    else:
        return usage()

    if sys.argv[2] == "auto":
        if sys.platform == 'win32':
            devpath = "COM5"
        else:
            devpath = "/dev/ttyACM0"
    else:
        devpath = sys.argv[2]

    s = fios_serial_open(devpath)

    if not s:
        return 1

    f = fios_file_send(s, sys.argv[3]) if sending else fios_file_receive(s, sys.argv[3])

    if not f:
        fios_serial_close(s)
        return 1

    sys.stdout.write("\n")
    sys.stdout.flush()

    while True:
        r = fios_file_idle(f)
        if r[0] != fios_file_status_in_progress:
            break
        sys.stdout.write("\rProgress: %.1f %%" % (r[1] * 100))
        sys.stdout.flush()
        time.sleep(0.1)

    sys.stdout.write("\n")
    sys.stdout.flush()

    fios_file_close(f)
    fios_serial_close(s)
    return 0

sys.exit(main())
