GCC = gcc
AS = as
RM = rm -rf

CFLAGS = -std=c99 
CFLAGS += -g
CFLAGS += -w

SRC_GCC = $(wildcard *.c)
SRC_ASM = X86_64cpuid.o

INC_PATH = ./

target = Asm_test.out

.PHYONE : out clean

out:$(SRC_ASM)
	$(GCC) -o $(target) $(SRC_GCC) $(SRC_ASM) -I$(INC_PATH) $(CFLAGS)

clean:
	$(RM) $(target)


%.o: %.s
	$(AS) $< -o $@ 