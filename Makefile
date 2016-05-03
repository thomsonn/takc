CFLAGS = `pkg-config --cflags glib-2.0`
CFLAGS += -ggdb -Wall -Wextra -std=gnu99 -D_GNU_SOURCE
CFLAGS += -Ofast -fomit-frame-pointer -march=native
#CFLAGS += -O0 -fno-omit-frame-pointer
LDLIBS = `pkg-config --libs glib-2.0` -lm
objects = takc.o bitboard.o fastlog.o

takc: $(objects)

bitboard: CFLAGS += -DTEST_BITBOARD
bitboard: bitboard.o

.PHONY: test
test: clean bitboard
	./bitboard
	make clean

.PHONY: clean
clean:
	rm -f takc $(objects)
