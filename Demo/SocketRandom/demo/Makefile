RUNDIR  = $(shell cd ../out && pwd)
CFLAGS = -std=gnu99
CFLAGS += -g
CFLAGS += -w
CFLAGS += -Wl,-rpath=$(RUNDIR)
CFLAGS += -L ../out -lrandom
CFLAGS += -I ../src -I ../src/algo  


SRC_SER=server.c
SRC_CLI=client.c

TARGET_CLIENT = client.out
TARGET_SERVER = server.out

.PHONY : clean

all:
	gcc $(SRC_CLI) -o $(TARGET_CLIENT) $(CFLAGS)
	gcc $(SRC_SER) -o $(TARGET_SERVER) $(CFLAGS) -pthread

clean:
	rm -rf *.out