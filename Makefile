SRC_DIR=src
OBJ_DIR=obj
INC_DIR=include
BIN_DIR=bin
DEPS_H=deps/isocline/include
DEPS_C=deps/isocline/src/isocline.c

target=native

ifeq ($(target), android)
ARCH=aarch64
TOOLCHAIN=$(NDK_PATH)/toolchains/llvm/prebuilt/linux-x86_64
TARGET=$(ARCH)-linux-android
API=28
CC=$(TOOLCHAIN)/bin/$(TARGET)$(API)-clang
CXX=$(TOOLCHAIN)/bin/$(TARGET)$(API)-clang++
else
CC=gcc
endif


CFLAGS=-g -Wall -I$(INC_DIR) -I$(DEPS_H)
LDFLAGS=

SRCS=$(wildcard $(SRC_DIR)/*.c) $(DEPS_C)
OBJS=$(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))
INCS=$(wildcard $(INC_DIR)/*.h) $(DEPS_H)

BINARY=cio

all: $(BIN_DIR)/$(BINARY)

android:

$(BIN_DIR)/$(BINARY): $(OBJS) $(BIN_DIR)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR)
	rm -rf $(OUTDIR)
