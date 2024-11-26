// SPDX-FileCopyrightText: 2024 Filipe Coelho <falktx@darkglass.com>
// SPDX-License-Identifier: ISC

#pragma once

#ifdef __cplusplus
extern "C" {
#else
#include <stdbool.h>
#include <stddef.h>
#endif

#ifdef USC_SHARED_LIBRARY
 #ifdef _WIN32
  #define USC_API __declspec(dllexport)
 #else
  #define USC_API __attribute__((visibility("default")))
 #endif
#else
 #define USC_API
#endif

#define CMD_SIZE 2 /* 'w ' */ + 10 /* 0xffffffff */ + 1 /* null */
#define MAX_PAYLOAD_SIZE 0x2000

// --------------------------------------------------------------------------------------------------------------------

typedef struct _fios_serial_t fios_serial_t;

USC_API
fios_serial_t* fios_serial_open(const char* devpath);

USC_API
void fios_serial_close(fios_serial_t* s);

USC_API
bool fios_serial_read_cmd(fios_serial_t* s, char cmd[CMD_SIZE]);

USC_API
bool fios_serial_read_payload(fios_serial_t* s, void* payload, size_t size);

// NOTE null-terminated string, maximum CMD_SIZE
USC_API
bool fios_serial_write_cmd(fios_serial_t* s, const char* cmd);

USC_API
bool fios_serial_write_payload(fios_serial_t* s, const void* payload, size_t size);

// --------------------------------------------------------------------------------------------------------------------

typedef struct _fios_file_t fios_file_t;

USC_API
fios_file_t* fios_file_receive(fios_serial_t* s, const char* outpath);

USC_API
fios_file_t* fios_file_send(fios_serial_t* s, const char* inpath);

USC_API
bool fios_file_idle(fios_file_t* s, float* progress);

USC_API
bool fios_file_close(fios_file_t* s);

// --------------------------------------------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif
