CC = gcc
CFLAGS = -Iinclude -Wall
LDFLAGS_COMMON = -lraylib

ifeq ($(OS),Windows_NT)
    LDFLAGS = -Llib/windows $(LDFLAGS_COMMON) -lopengl32 -lgdi32 -lwinmm -static
    EXT = .exe
    CLEAN_CMD = del *.exe
else
    LDFLAGS = lib/linux/libraylib.a -lGL -lm -lpthread -ldl -lrt -lX11
    EXT = .out
    CLEAN_CMD = rm -f $(TARGET) $(TEST_BINS)
endif

SRC = src/graph.c src/io.c src/gui_extras.c src/physics.c
TEST_FILES = $(wildcard test/*.c)
TEST_BINS = $(TEST_FILES:test/%.c=%$(EXT))
TARGET = app$(EXT)

all: $(TARGET) tests

$(TARGET): src/main.c $(SRC)
	$(CC) src/main.c $(SRC) -o $(TARGET) $(CFLAGS) $(LDFLAGS)

tests: $(TEST_BINS)

%$(EXT): test/%.c $(SRC)
	$(CC) $< $(SRC) -o $@ $(CFLAGS) $(LDFLAGS)

clean:
	$(CLEAN_CMD)
