// SPDX-FileCopyrightText: 2024 Filipe Coelho <falktx@darkglass.com>
// SPDX-License-Identifier: ISC

#pragma once

#include "libfios.h"

#ifdef _WIN32
typedef void* HANDLE;
#endif

typedef struct _fios_serial_t {
    char* devpath;
   #ifdef _WIN32
    HANDLE h;
   #else
    int fd;
   #endif
} fios_serial_t;
