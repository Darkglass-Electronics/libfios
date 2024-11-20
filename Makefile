
CFLAGS += -Wall -Wextra -O0 -g

all: serial-test serial-test-arm64 serial-test-win64.exe

serial-test: main.c
	$(CC) $< $(CFLAGS) $(LDFLAGS) -o $@

serial-test-arm64: main.c
	aarch64-linux-gnu-gcc $< $(CFLAGS) $(LDFLAGS) -o $@

serial-test-win64.exe: main.c
	x86_64-w64-mingw32-gcc $< -static -mstackrealign -o $@
