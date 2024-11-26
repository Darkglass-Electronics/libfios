// SPDX-FileCopyrightText: 2024 Filipe Coelho <falktx@darkglass.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "libfios.h"

#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

static int usage(char* argv[])
{
    fprintf(stderr, "Usage: %s [r|s] [device-path|auto] [file-path]\n", argv[0]);
    return 1;
}

int main(int argc, char* argv[])
{
    if (argc <= 3)
        return usage(argv);

    bool sending;
    if (!strcmp(argv[1], "r"))
        sending = false;
    else if (!strcmp(argv[1], "s"))
        sending = true;
    else
        return usage(argv);

    const char* devpath;
    if (!strcmp(argv[2], "auto"))
    {
       #if defined(_WIN32)
        devpath = "COM5";
       #elif defined(__aarch64__)
        devpath = "/dev/ttyGS0";
       #else
        devpath = "/dev/ttyACM0";
       #endif
    }
    else
    {
        devpath = argv[2];
    }

    fios_serial_t* const s = fios_serial_open(devpath);

    if (s == NULL)
        return 1;

    fios_file_t* const f = sending ? fios_file_send(s, argv[3])
                                   : fios_file_receive(s, argv[3]);

    if (f == NULL)
    {
        fios_serial_close(s);
        return 1;
    }

    fprintf(stdout, "\n");
    fflush(stdout);

    float progress;
    while (fios_file_idle(f, &progress))
    {
        fprintf(stdout, "\rProgress: %.1f %%", progress * 100);
        fflush(stdout);
       #ifdef _WIN32
        Sleep(100);
       #else
        usleep(100 * 1000);
       #endif
    }

    fprintf(stdout, "\n");
    fflush(stdout);

    fios_file_close(f);
    fios_serial_close(s);
    return 0;
}
