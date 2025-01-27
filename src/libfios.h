// SPDX-FileCopyrightText: 2024 Filipe Coelho <falktx@darkglass.com>
// SPDX-License-Identifier: ISC

#pragma once

#ifdef __cplusplus
extern "C" {
#else
#include <stdbool.h>
#include <stddef.h>
#endif

/*! define FIOS_API depending on the build type
 */
#ifdef FIOS_SHARED_LIBRARY
 #ifdef _WIN32
  #define FIOS_API __declspec(dllexport)
 #else
  #define FIOS_API __attribute__((visibility("default")))
 #endif
#else
 #define FIOS_API
#endif

/*! use a well known size for commands, giving enough space for a small single argument
 */
#define CMD_SIZE 2 /* 'w ' */ + 10 /* 0xffffffff */ + 1 /* null */

/*! maximum size allowed in file APIs
 */
#define MAX_FILE_SIZE 0x7fffffff

/*! maximum payload size, used to receive data after a command
 */
#define MAX_PAYLOAD_SIZE 0x2000

/*! opaque API structures
 */
typedef struct _fios_serial_t fios_serial_t;
typedef struct _fios_file_t fios_file_t;

// --------------------------------------------------------------------------------------------------------------------
// serial IO

/*! Open the serial port at @a devpath
 */
FIOS_API
fios_serial_t* fios_serial_open(const char* devpath);

/*! Close a serial port
 */
FIOS_API
void fios_serial_close(fios_serial_t* s);

// --------------------------------------------------------------------------------------------------------------------
// serial communication

/*! read CMD_SIZE bytes from a serial port into @a cmd
 * @note this is a blocking operation
 */
FIOS_API
bool fios_serial_read_cmd(fios_serial_t* s, char cmd[CMD_SIZE]);

/*! read up to MAX_PAYLOAD_SIZE bytes from a serial port into @a payload
 * the @a payload argument must have enough space to receive the data
 * @note this is a blocking operation
 */
FIOS_API
bool fios_serial_read_payload(fios_serial_t* s, void* payload, size_t size);

/*! write up to CMD_SIZE bytes from @a cmd to a serial port
 * the @a cmd argument can be less than CMD_SIZE, in which case the remaining bytes will be zero
 * @note this is a blocking operation
 */
FIOS_API
bool fios_serial_write_cmd(fios_serial_t* s, const char* cmd);

/*! write @a size bytes from @a payload to a serial port
 * @note this is a blocking operation
 */
FIOS_API
bool fios_serial_write_payload(fios_serial_t* s, const void* payload, size_t size);

// --------------------------------------------------------------------------------------------------------------------
// file operations (using background threads)

typedef enum {
    fios_file_status_error,
    fios_file_status_in_progress,
    fios_file_status_completed,
} fios_file_status_t;

/*! prepare to receive data from a serial port into the file @a outpath
 * a background thread is used for receiving data from the serial port and writing to the file
 * use @fios_file_idle to query current progress and @fios_file_close when done
 */
FIOS_API
fios_file_t* fios_file_receive(fios_serial_t* s, const char* outpath);

/*! prepare to send data from the file @a inpath into a serial port
 * a background thread is used for reading the file and sending data to the serial port
 * use @fios_file_idle to query current progress and @fios_file_close when done
 */
FIOS_API
fios_file_t* fios_file_send(fios_serial_t* s, const char* inpath);

/*! check status of an active serial file transfer
 * when passing a valid @a progress pointer it will indicate current progress between 0.0 and 1.0
 * returns true if the file is still being received/sent, false if operation completed or failed
 */
FIOS_API
fios_file_status_t fios_file_idle(fios_file_t* f, float* progress);

/*! get the current progress of an active serial file transfer
 * returns a value between 0.0 and 1.0
 */
FIOS_API
float fios_file_get_progress(fios_file_t* f);

/*! close the file operation
 * must still be called even if @fios_file_idle returns false
 */
FIOS_API
void fios_file_close(fios_file_t* f);

/*! get the error message for the case where @fios_file_idle returns @fios_file_status_error
 * must not be called after @fios_file_close
 */
FIOS_API
const char* fios_file_get_last_error(fios_file_t* f);

// --------------------------------------------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif
