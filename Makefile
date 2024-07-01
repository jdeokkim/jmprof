#
# Copyright (c) 2024 Jaedeok Kim <jdeokkim@protonmail.com>
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#

# ============================================================================>

.POSIX:

# ============================================================================>

.PHONY: all clean

# ============================================================================>

_COLOR_BEGIN = \033[1;38;5;026m
_COLOR_END = \033[m

# ============================================================================>

PROJECT_NAME = jmprof
PROJECT_FULL_NAME = jdeokkim/jmprof

PROJECT_PREFIX = ${_COLOR_BEGIN}${PROJECT_FULL_NAME}:${_COLOR_END}

# ============================================================================>

BINARY_PATH = bin
INCLUDE_PATH = include
LIBRARY_PATH = lib
SOURCE_PATH = src

OBJECTS = \
	${SOURCE_PATH}/backtrace.o  \
	${SOURCE_PATH}/preload.o    \
	${SOURCE_PATH}/printf.o

TARGETS = ${LIBRARY_PATH}/lib${PROJECT_NAME}.so

PREFIX = ${DESTDIR}/usr

# ============================================================================>

CC = cc
CFLAGS = -D_DEFAULT_SOURCE -fPIC -g -I${INCLUDE_PATH} -O2 -std=gnu99
LDFLAGS = -lpthread -lunwind -lunwind-generic -pthread -shared

CFLAGS += -Wall -Wno-unused-but-set-variable -Wno-unused-value \
	-Wno-unused-variable

# ============================================================================>

all: pre-build build post-build

pre-build:
	@printf "${PROJECT_PREFIX} Using: '${CC}' to build this project.\n"

build: ${TARGETS}

.c.o:
	@printf "${PROJECT_PREFIX} Compiling: $@ (from $<)\n"
	@${CC} -c $< -o $@ ${CFLAGS}

${TARGETS}: ${OBJECTS}
	@mkdir -p ${LIBRARY_PATH}
	@printf "${PROJECT_PREFIX} Linking: ${TARGETS}\n"
	@${CC} ${OBJECTS} -o ${TARGETS} ${LDFLAGS} ${LDLIBS}

post-build:
	@printf "${PROJECT_PREFIX} Build complete.\n"

superuser:
	@if [ "$(shell id -u)" -ne 0 ]; then                       \
		printf "${PROJECT_PREFIX} You must be superuser to ";  \
		printf "install or uninstall this library.\n";         \
		exit 1;                                                \
	fi

install: superuser
	@printf "${PROJECT_PREFIX} Installing: "
	@printf "${PREFIX}/${BINARY_PATH}/${PROJECT_NAME}\n"
	@install -m755 ${BINARY_PATH}/${PROJECT_NAME} \
		${PREFIX}/${BINARY_PATH}/${PROJECT_NAME}
	@printf "${PROJECT_PREFIX} Installing: ${PREFIX}/${TARGETS}\n"
	@install -m644 ${TARGETS} ${PREFIX}/${TARGETS} && ldconfig

uninstall: superuser
	@printf "${PROJECT_PREFIX} Uninstalling: ${PREFIX}/bin/${PROJECT_NAME}\n"
	@rm -f ${PREFIX}/bin/${PROJECT_NAME}
	@printf "${PROJECT_PREFIX} Uninstalling: ${PREFIX}/${TARGETS}\n"
	@rm -f ${PREFIX}/${TARGETS}

clean:
	@printf "${PROJECT_PREFIX} Cleaning up.\n"
	@rm -f ${LIBRARY_PATH}/*.so ${SOURCE_PATH}/*.o
