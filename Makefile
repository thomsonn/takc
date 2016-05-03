CFLAGS = `pkg-config --cflags glib-2.0`
CFLAGS += -ggdb -Wall -Wextra
CFLAGS += -Ofast -fomit-frame-pointer -march=native
#CFLAGS += -O0 -fno-omit-frame-pointer
LDLIBS = `pkg-config --libs glib-2.0` -lm
objects = takc.o bitboard.o

takc: $(objects)

bitboard: CFLAGS += -DTEST_BITBOARD
bitboard: bitboard.o

clean:
	rm -f takc $(objects)
