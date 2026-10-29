# Pac-Man en C - Makefile (MinGW en Windows: usar mingw32-make)
#   make           -> version consola (pacman)
#   make gui       -> version grafica raylib (pacman_gui). Definir RAYLIB_DIR si hace falta.
#   make test      -> tests del nucleo

CC       ?= gcc
CFLAGS   ?= -std=c11 -Wall -Wextra -Wpedantic -O2
CPPFLAGS := -Iinclude

CORE_SRC     := $(wildcard src/core/*.c)
VIEW_SRC     := $(wildcard src/view/*.c)
CONSOLE_SRC  := $(wildcard src/platform/*.c)
RAYLIB_SRC   := src/platform/platform_clock.c $(wildcard src/platform/raylib/*.c)

APP_SRC      := src/main.c $(CORE_SRC) $(VIEW_SRC) $(CONSOLE_SRC)
GUI_SRC      := src/main.c $(CORE_SRC) $(VIEW_SRC) $(RAYLIB_SRC)
TEST_SRC     := tests/test_main.c $(CORE_SRC) $(VIEW_SRC)

RAYLIB_DIR   ?= C:/raylib/raylib/src
ifneq ($(wildcard $(RAYLIB_DIR)/include/raylib.h),)
    RL_INC := $(RAYLIB_DIR)/include
    RL_LIB := $(RAYLIB_DIR)/lib
else
    RL_INC := $(RAYLIB_DIR)
    RL_LIB := $(RAYLIB_DIR)
endif

ifeq ($(OS),Windows_NT)
    EXE        := .exe
    RM_CMD     := del /Q
    RAYLIB_LDLIBS := -lraylib -lopengl32 -lgdi32 -lwinmm -mwindows
else
    EXE        :=
    RM_CMD     := rm -f
    RAYLIB_LDLIBS := -lraylib -lGL -lm -lpthread -ldl -lX11
endif

TARGET      := pacman$(EXE)
GUI_TARGET  := pacman_gui$(EXE)
TEST_TARGET := pacman_tests$(EXE)

.PHONY: all gui run run-gui test clean

all: $(TARGET)

gui: $(GUI_TARGET)

$(TARGET): $(APP_SRC)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(APP_SRC) -o $@

$(GUI_TARGET): $(GUI_SRC)
	$(CC) $(CPPFLAGS) -I$(RL_INC) $(CFLAGS) $(GUI_SRC) -L$(RL_LIB) $(RAYLIB_LDLIBS) -o $@

$(TEST_TARGET): $(TEST_SRC)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(TEST_SRC) -o $@

run: $(TARGET)
	./$(TARGET)

run-gui: $(GUI_TARGET)
	./$(GUI_TARGET)

test: $(TEST_TARGET)
	./$(TEST_TARGET)

clean:
	-$(RM_CMD) $(TARGET) $(GUI_TARGET) $(TEST_TARGET)
