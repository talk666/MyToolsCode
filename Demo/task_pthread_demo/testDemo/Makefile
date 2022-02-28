CFLAGS += -std=gnu99 -w -O0 -g -lrt -pthread
SRC_OBJ += $(wildcard capbenchmark/*.c)
all :
	gcc $(CFLAGS) -o test $(SRC_OBJ)
clean:
	rm -f test 
