takc: main.c
	gcc -Wall -Wextra -O1 -ggdb -lm -o takc main.c

all:
	takc

clean:
	rm takc
