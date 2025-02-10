CC=gcc
CSA=scan-build

CFLAGS = -c -std=c99 -Wall -Wextra -ggdb3
SDL_CFLAGS = $(shell pkg-config --cflags sdl2)
LDLIBS= $(shell pkg-config --libs sdl2)

override CFLAGS += $(SDL_CFLAGS)

SOURCES = $(shell find src -name "*.c")
HEADER_FILES = $(shell find src -name "*.h")
OBJECTS = $(SOURCES:.c=.o)
BUILD_DIR = build

TARGET=chip8

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) $(LDLIBS) -o $(TARGET)
	@mkdir -p $(BUILD_DIR)
	@mv $(OBJECTS) $(BUILD_DIR)

%.o: %.c $(HEADER_FILES)
	$(CC) $(CFLAGS) -o2 -o $@ $<

dbg: $(TARGET)
	./$(TARGET) ./test-roms/test_opcode.ch8 DEBUG

csa:
	$(CSA) $(CC) $(CFLAGS) $(SOURCES)

clean:
	rm -rf src/*.o $(TARGET)
