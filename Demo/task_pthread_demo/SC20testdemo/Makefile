dir = $(shell cd ../../.. && pwd)

LIBS = -L ../../../deploy -lSC20api -lZaykKey -lpthread
INCS = -I ../../../deploy

CFLAGS += -Wl,-rpath=$(dir)/deploy

.PHONY:all
all :
	gcc -g $(CFLAGS) -w -O0 -o zayk_capbenchmark.out $(wildcard Zayk_capbenchmark/*.c)  $(LIBS) $(INCS) -lrt -pthread -std=gnu99
clean:
	rm -f zayk_capbenchmark.out
