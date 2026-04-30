// SPDX-FileCopyrightText: 2024-2026 Filipe Coelho <falktx@darkglass.com>
// SPDX-License-Identifier: ISC

#pragma once

#include "libfios.h"

#ifdef __cplusplus
#include <cstddef>
extern "C" {
#include <stddef.h>
#endif

typedef size_t libfios_stream_read(void* buffer, size_t size, size_t n, void* cookie);
typedef size_t libfios_stream_write(const void* buffer, size_t size, size_t n, void* cookie);
typedef int libfios_stream_close(void* cookie);

typedef struct _libfios_stream_functions {
    libfios_stream_read* read;
    libfios_stream_write* write;
    libfios_stream_close* close;
} libfios_stream_functions;

fios_file_t* fios_file_send_stream(fios_serial_t* s, long size, libfios_stream_functions funcs, void* cookie);

#ifdef __cplusplus
}
#endif
