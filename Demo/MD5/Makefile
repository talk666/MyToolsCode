
CFLAGS += -w -std=gnu99 -g
#指定运行时动态库路径

RUNDIR = ./
SRC_FILES += $(wildcard ./*.c)

CC = gcc

INCS = -I$(RUNDIR)

TARGET = md5demo

.PHONY:$(TARGET) clean

$(TARGET):$(SRC_FILES)
	$(CC) -o $(TARGET) $(CFLAGS) $^  $(INCS)
	rm *.o -f

clean:
	rm $(TARGET) -f
