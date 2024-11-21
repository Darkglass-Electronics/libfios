// SPDX-FileCopyrightText: 2024 Filipe Coelho <falktx@darkglass.com>
// SPDX-License-Identifier: ISC

#pragma once

#include <stdbool.h>
#include <stddef.h>

#ifdef _WIN32
typedef void* HANDLE;
#endif

typedef struct {
   #ifdef _WIN32
    HANDLE h;
   #else
    int fd;
   #endif
} usc_serial_t;

#define CMD_SIZE 2 /* 'w ' */ + 10 /* 0xffffffff */ + 1 /* ' ' */ + 1 /* null */
#define MAX_PAYLOAD_SIZE 0x2000

usc_serial_t* usc_serial_open(const char* devpath);

void usc_serial_close(usc_serial_t* s);

bool usc_serial_read_cmd(usc_serial_t* s, char cmd[CMD_SIZE]);

bool usc_serial_read_payload(usc_serial_t* s, void* payload, size_t size);

// NOTE null-terminated string, maximum CMD_SIZE
bool usc_serial_write_cmd(usc_serial_t* s, const char* cmd);

bool usc_serial_write_payload(usc_serial_t* s, const void* payload, size_t size);
