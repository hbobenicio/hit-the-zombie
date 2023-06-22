# Explicitly defining a default general compiler here because
# then it defaults to using `cc` which is a symbolic link (with probably multiple indirections)
# to the actual compiler. This is fine for the project to compile but makes it harder
# to identify the compiler in this Makefile so we could customize specific flags to it.
CC = gcc

SRC = \
	$(wildcard src/hit-the-zoombie/util/*.c) \
	$(wildcard src/hit-the-zoombie/game/*.c) \
	$(wildcard src/hit-the-zoombie/*.c)
OBJ = $(SRC:.c=.o)
DEP = $(OBJ:.o=.d)
BIN = hit-the-zoombie

PKG_CFLAGS := $(shell pkg-config --cflags sdl2 SDL2_image SDL2_ttf SDL2_mixer)
PKG_LDLIBS := $(shell pkg-config --libs   sdl2 SDL2_image SDL2_ttf SDL2_mixer)

# MMD will generate a .d file for each .c module containing its Makefile rules (including dependencies)
# These .d files are used with the `-include` bellow
CFLAGS  = -Wall -Wextra -Wpedantic -std=c17 -MMD -I ./src/ -I ./vendor/stb/0.67/ $(PKG_CFLAGS)
LDFLAGS = -Wall -Wextra -Wpedantic -std=c17
LDLIBS  = $(PKG_LDLIBS)

.PHONY: all release debug profile run install distclean clean

all: debug

release: CFLAGS  += -O3 -march=native
release: LDFLAGS += -O3 -march=native
release: $(BIN)
	strip $(BIN)

ifeq ($(CC),clang)
debug: CFLAGS  += -fsanitize=address,undefined -fno-omit-frame-pointer
debug: LDFLAGS += -fsanitize=address,undefined -fno-omit-frame-pointer
endif

ifeq ($(CC),gcc)
debug: CFLAGS  += -fsanitize=address,undefined -fno-omit-frame-pointer
debug: LDFLAGS += -fsanitize=address,undefined -fno-omit-frame-pointer
endif

debug: CFLAGS  += -g -O0 -DDEBUG
debug: LDFLAGS += -g -O0
debug: $(BIN)

ifeq ($(CC),gcc)
profile: CFLAGS  += -g -pg -no-pie -fno-builtin
profile: LDFLAGS += -g -pg -no-pie -fno-builtin
endif

profile: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

-include $(DEP)

run: $(BIN)
	./$(BIN)

# FIXME: Not working atm (we need to install assets too). Probably better to package it into a deb file first
install:
	install $(BIN) $(HOME)/bin/

distclean: clean
	$(RM) -f $(BIN)

clean:
	find . \
		-type f -name '*.d' -delete -o \
		-type f -name '*.o' -delete
