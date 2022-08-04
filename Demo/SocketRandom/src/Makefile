SRCS = $(wildcard ./*.c)
SRCS += $(wildcard algo/*.c)

LIB = librandom.so

OUT = $(shell pwd)/../out

CFLAGS = -g -I. -Ialgo

.PHONY: all
all:
	rm -rf $(OUT)
	mkdir $(OUT)
	$(CC) -shared -fpic -o $(LIB) $(SRCS) $(CFLAGS)
	cp -rf $(LIB) $(OUT)
	cp -rf $(LIB) $(shell pwd)/..

clean:
	rm -rf $(OUT)
