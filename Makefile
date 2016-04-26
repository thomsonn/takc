takc: main.c
	gcc -ggdb -lm -O3 -o takc main.c

all:
	takc

clean:
	rm takc
