CFLAGS = `pkg-config --cflags glib-2.0` -ggdb -Wall -Wextra -O1
LDADD = `pkg-config --libs glib-2.0`
objects = takc.o

takc: $(objects)

clean:
	rm -f takc
