SRC = \
	$(wildcard src/snake/game/*.c) \
	$(wildcard src/snake/*.c)
OBJ = $(SRC:.c=.o)
DEP = $(OBJ:.o=.d)
BIN = snake

PKG_CFLAGS := $(shell pkg-config --cflags sdl2 SDL2_image SDL2_ttf SDL2_mixer)
PKG_LDLIBS := $(shell pkg-config --libs sdl2 SDL2_image SDL2_ttf SDL2_mixer)

# MMD will generate a .d file for each .c module containing its Makefile rules (including dependencies)
# These .d files are used with the `-include` bellow
CFLAGS  = -Wall -Wextra -pedantic -std=c17 -MMD -I ./src/ $(PKG_CFLAGS)
LDFLAGS = -Wall -Wextra -pedantic -std=c17
LDLIBS  = $(PKG_LDLIBS)

.PHONY: all release debug distclean clean

all: debug

release: CFLAGS += -O3
release: LDFLAGS += -O3
release: $(BIN)
	strip $(BIN)

release-static: CFLAGS += -static
release-static: LDFLAGS += -static
release-static: $(BIN_STATIC)

ifeq ($(CC),clang)
debug: CFLAGS += -fsanitize=address
debug: LDFLAGS += -fsanitize=address
endif

ifeq ($(CC),gcc)
debug: CFLAGS += -fsanitize=address
debug: LDFLAGS += -fsanitize=address
endif

debug: CFLAGS += -g -O0 -DDEBUG
debug: LDFLAGS += -g -O0
debug: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

-include $(DEP)

distclean: clean
	rm -f $(BIN)

clean:
	find . \
		-type f -name '*.d' -delete -o \
		-type f -name '*.o' -delete
