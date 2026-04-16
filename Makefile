CC = gcc
CFLAGS = -Iinclude -Wall -g

ifeq ($(OS),Windows_NT)
    LDFLAGS = -Llib/windows -lraylib -lopengl32 -lgdi32 -lwinmm -static
    EXT = .exe
	CLEAN_CMD = del /Q $(TARGET) $(subst /,\,$(TEST_BINS))
else
    LDFLAGS = lib/linux/libraylib.a -lGL -lm -lpthread -ldl -lrt -lX11
    EXT = .out
    CLEAN_CMD = rm -f $(TARGET) $(TEST_BINS)
endif

GUI_SRC = src/physics.c src/gui_interface.c
LOGIC_SRC = src/graph.c src/io.c
TEST_FILES = $(wildcard tests/*.c)
TEST_BINS = $(TEST_FILES:tests/%.c=%$(EXT))
TARGET = app$(EXT)

all: $(TARGET)

$(TARGET): src/main.c $(GUI_SRC) $(LOGIC_SRC)
	$(CC) src/main.c $(GUI_SRC) $(LOGIC_SRC) -o $(TARGET) $(CFLAGS) $(LDFLAGS)

tests: $(TEST_BINS)

%$(EXT): tests/%.c $(LOGIC_SRC)
	$(CC) $< $(LOGIC_SRC) -o $@ $(CFLAGS) -lm -DLOGIC_ONLY

clean:
	$(CLEAN_CMD)
