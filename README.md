# libfios

This is a small C library for reading/writing arbitrary data to and from a serial port.

See [src/libfios.h](src/libfios.h) for the API.

## Building

libfios uses cmake for building and has no other dependencies (besides a working C compiler).

building libfios is as simple as:

```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## Example

### Standalone

An example standalone utility is included as part of the default build, called "fios-file".

```
$ ./build/fios-file
Usage: ./build/fios-file [r|s] [device-path|auto] [file-path]
```

The 1st argument is either "r" for "receive" or "s" for "send".  
The 2nd argument specifies the serial port to use (e.g. `/dev/ttyUSB0` on Linux and `COM5` on Windows)  
The 3rd argument specifies the file to read or write (dependending on the receive vs send mode)

### Code

Here is a small example on how to use this library to send a binary file.

Receiver:

```
fios_serial_t* const s = fios_serial_open("/dev/ttyUSB0");
assert(s);
fios_file_t* const f = fios_file_receive(s, "/tmp/test.bin");
assert(f);
while (fios_file_idle(f, NULL))
  sleep(1);
fios_file_close(f);
fios_serial_close(s);
```

Sender:

```
fios_serial_t* const s = fios_serial_open("/dev/ttyUSB1");
assert(s);
fios_file_t* const f = fios_file_send(s, "/path/to/bin.file");
assert(f);
while (fios_file_idle(f, NULL))
  sleep(1);
fios_file_close(f);
fios_serial_close(s);
```
