// SPDX-FileCopyrightText: 2024 Filipe Coelho <falktx@darkglass.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "libusc.h"

#undef NDEBUG
#define DEBUG
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEBUG_PRINT(...)
// #define DEBUG_PRINT(...) printf(__VA_ARGS__)

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
        devpath = "COM3";
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

    usc_serial_t* const s = usc_serial_open(devpath);
    assert(s != NULL);

    char buf[MAX_PAYLOAD_SIZE];
    char cmd[CMD_SIZE];

    bool test;
    if (sending)
    {
        FILE* const in = fopen(argv[3], "r");
        assert(in);
        fseek(in, 0, SEEK_SET);

        for (;;)
        {
            const int r = fread(buf, 1, MAX_PAYLOAD_SIZE, in);

            DEBUG_PRINT("main file read return %d | 0x%x\n", r, r);

            if (r <= 0)
                break;

            DEBUG_PRINT("writing command for %d | 0x%x bytes\n", r, r);

            // encode write command as first byte, followed by expected size, and then the payload
            snprintf(cmd, CMD_SIZE, "w 0x%04x", r);

            test = usc_serial_write_cmd(s, cmd);
            assert(test);

            DEBUG_PRINT("writing payload for %d | 0x%x bytes\n", r, r);
            test = usc_serial_write_payload(s, buf, r);
            assert(test);

            DEBUG_PRINT("waiting for ok signal for %d | 0x%x bytes\n", r, r);
            test = usc_serial_read_cmd(s, cmd);
            assert(test);
        }

        DEBUG_PRINT("writing command for close\n");
        test = usc_serial_write_cmd(s, "q");
        assert(test);

        fclose(in);
    }
    else
    {
        FILE* out = fopen(argv[3], "w");
        assert(out);
        fseek(out, 0, SEEK_SET);

        for (;;)
        {
            DEBUG_PRINT("waiting for command\n");

            test = usc_serial_read_cmd(s, cmd);
            assert(test);

            if (cmd[0] == 'q' && cmd[1] == 0)
                break;

            if (cmd[0] != 'w' || cmd[1] != ' ')
            {
                fprintf(stderr, "error invalid command type %02x:'%c' %02x:'%c'", cmd[0], cmd[0], cmd[1], cmd[1]);
                break;
            }

            cmd[CMD_SIZE - 1] = 0;

            // size comes as 2nd arg
            long int size = strtol(cmd + 2, NULL, 16);

            DEBUG_PRINT("waiting for payload of size %ld | 0x%04lx\n", size, size);
            test = usc_serial_read_payload(s, buf, size);
            assert(test);

            DEBUG_PRINT("payload received, sending ok back\n");
            test = usc_serial_write_cmd(s, "ok");
            assert(test);

            // write received buffer to file
            for (int w = 0, total = 0; total < size; total += w)
            {
                w = fwrite(buf + total, 1, size - total, out);

                if (w < 0)
                {
                    perror("error partial write");
                    break;
                }
            }
        }

        fclose(out);
    }

    usc_serial_close(s);
    return 0;
}
