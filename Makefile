CXX = gcc

SRC = src/main.c
INCLUDE = include
TARGET = app

ifeq ($(OS),Windows_NT)
	LDFLAGS = -Llib/windows -lraylib -lopengl32 -lgdi32 -lwinmm -static
	TARGET_EXEC = $(TARGET).exe
	CLEAN_CMD = del $(TARGET_EXEC)
else
	LDFLAGS = lib/linux/libraylib.a -lGL -lm -lpthread -ldl -lrt -lX11
	TARGET_EXEC = $(TARGET)
	CLEAN_CMD = rm -f $(TARGET_EXEC)
endif

CXXFLAGS = -I$(INCLUDE) -Wall

all: $(TARGET_EXEC)

$(TARGET_EXEC): $(SRC)
	$(CXX) $(SRC) -o $(TARGET_EXEC) $(CXXFLAGS) $(LDFLAGS)

clean:
	$(CLEAN_CMD)
