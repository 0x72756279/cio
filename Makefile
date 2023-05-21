SRC_DIR=src
OBJ_DIR=obj
INC_DIR=include
BIN_DIR=bin
DEPS_H=deps/isocline/include
DEPS_C=deps/isocline/src/isocline.c

VERSION = 0.0.1

# paths
PREFIX = /usr/local
MANPREFIX = ${PREFIX}/share/man

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

ifneq ($(OS),Windows_NT)
	OS=$(shell uname -s)
endif


CFLAGS=-Wall -I$(INC_DIR) -I$(DEPS_H)
LDFLAGS=

SRCS=$(wildcard $(SRC_DIR)/*.c) $(DEPS_C)
OBJS=$(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))
INCS=$(wildcard $(INC_DIR)/*.h) $(DEPS_H)

BINARY=cio

all: CFLAGS+= -g -fsanitize=address -DDEBUG
all: options $(BIN_DIR)/$(BINARY)

release: CFLAGS+=-s -O3
release: options $(BIN_DIR)/$(BINARY)

options:
	@echo ${BINARY} build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"
	@echo "OS       = ${OS}"

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

install: all
ifneq ($(OS),Windows_NT)
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp -f ${BIN_DIR}/${BINARY} ${DESTDIR}${PREFIX}/bin
	chmod 755 ${DESTDIR}${PREFIX}/bin/${BINARY}
	mkdir -p ${DESTDIR}${MANPREFIX}/man1
	sed "s/VERSION/${VERSION}/g" < ${BINARY}.1 > ${DESTDIR}${MANPREFIX}/man1/${BINARY}.1
	chmod 644 ${DESTDIR}${MANPREFIX}/man1/${BINARY}.1
endif
