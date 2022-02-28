CC = gcc
CFLAGS = -g  -std=gnu99

OBJ = $(wildcard ./*.c)
OBJ += $(wildcard miracl/*.c)

target = test.out
.PHONY:all clean
all:
	$(CC) $(CFLAGS) $(OBJ) -o $(target)
	rm -rf ./*.o miracl/*.o
clean:
	rm -rf $(target)

