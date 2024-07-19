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

_COLOR_BEGIN = \033[1;38;5;208m
_COLOR_END = \033[m

# ============================================================================>

PROJECT_NAME = jmprof
PROJECT_FULL_NAME = jdeokkim/jmprof

PROJECT_VERSION = "0.0.6-dev"

LOG_PREFIX = ${_COLOR_BEGIN}${PROJECT_FULL_NAME}:${_COLOR_END}

# ============================================================================>

BINARY_PATH = bin
INCLUDE_PATH = include
LIBRARY_PATH = lib
SOURCE_PATH = src

# ============================================================================>

PREFIX = ${DESTDIR}/usr

OBJECTS_B1 = \
	${SOURCE_PATH}/interpret.o

OBJECTS_L1= \
	${SOURCE_PATH}/backtrace.o  \
	${SOURCE_PATH}/preload.o    \
	${SOURCE_PATH}/printf.o     \
	${SOURCE_PATH}/tracker.o

TARGET_B1 = ${BINARY_PATH}/${PROJECT_NAME}-ip
TARGET_L1 = ${LIBRARY_PATH}/lib${PROJECT_NAME}.so

# ============================================================================>

CC = cc
CFLAGS = -D_DEFAULT_SOURCE -fPIC -g \
	-I${INCLUDE_PATH} -I${SOURCE_PATH}/external -O1 -std=gnu99

# CFLAGS += -Wall -Wpedantic

LDFLAGS_B1 = 
LDFLAGS_L1 = -pthread -shared

LDLIBS_B1 = -ldw -lelf
LDLIBS_L1 = -lpthread -lunwind

# ============================================================================>

all: pre-build build post-build

pre-build:
	@printf "${LOG_PREFIX} Using: '${CC}' to build this project.\n"
	@sed -i "s/readonly version=.*/readonly version=\"${PROJECT_VERSION}\"/" \
		${BINARY_PATH}/${PROJECT_NAME}
	@sed -i "s/JMPROF_VERSION .*/JMPROF_VERSION       \"${PROJECT_VERSION}\"/" \
		${INCLUDE_PATH}/jmprof.h

build: ${TARGET_B1} ${TARGET_L1}

.c.o:
	@printf "${LOG_PREFIX} Compiling: $@ (from $<)\n"
	@${CC} -c $< -o $@ ${CFLAGS}

${TARGET_B1}: ${OBJECTS_B1}
	@mkdir -p ${BINARY_PATH}
	@printf "${LOG_PREFIX} Linking: ${TARGET_B1}\n"
	@${CC} ${OBJECTS_B1} -o ${TARGET_B1} ${LDFLAGS_B1} ${LDLIBS_B1}

${TARGET_L1}: ${OBJECTS_L1}
	@mkdir -p ${LIBRARY_PATH}
	@printf "${LOG_PREFIX} Linking: ${TARGET_L1}\n"
	@${CC} ${OBJECTS_L1} -o ${TARGET_L1} ${LDFLAGS_L1} ${LDLIBS_L1}

post-build:
	@printf "${LOG_PREFIX} Build complete.\n"

fresh: clean all install

superuser:
	@if [ "$(shell id -u)" -ne 0 ]; then                   \
		printf "${LOG_PREFIX} You must be superuser to ";  \
		printf "install or uninstall this library.\n";     \
		exit 1;                                            \
	fi

install: superuser
	@printf "${LOG_PREFIX} Installing: "
	@printf "${PREFIX}/${BINARY_PATH}/${PROJECT_NAME}\n"
	@install -m755 ${BINARY_PATH}/${PROJECT_NAME} \
		${PREFIX}/${BINARY_PATH}/${PROJECT_NAME}
	@printf "${LOG_PREFIX} Installing: "
	@printf "${PREFIX}/${TARGET_B1}\n"
	@install -m755 ${TARGET_B1} \
		${PREFIX}/${TARGET_B1}
	@printf "${LOG_PREFIX} Installing: ${PREFIX}/${TARGET_L1}\n"
	@install -m644 ${TARGET_L1} ${PREFIX}/${TARGET_L1} && ldconfig

uninstall: superuser
	@printf "${LOG_PREFIX} Uninstalling: "
	@printf "${PREFIX}/${BINARY_PATH}/${PROJECT_NAME}\n"
	@rm -f ${PREFIX}/${BINARY_PATH}/${PROJECT_NAME}
	@printf "${LOG_PREFIX} Uninstalling: ${PREFIX}/${TARGET_B1}\n"
	@rm -f ${PREFIX}/${TARGET_B1}
	@printf "${LOG_PREFIX} Uninstalling: ${PREFIX}/${TARGET_L1}\n"
	@rm -f ${PREFIX}/${TARGET_L1}

clean:
	@printf "${LOG_PREFIX} Cleaning up.\n"
	@rm -f ${LIBRARY_PATH}/*.so ${SOURCE_PATH}/*.o
