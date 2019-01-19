# Makefile for 6502
# written by hyan23
# Date: 2017.08.02

CCFLAGS		=		-O3
OBJECTS		=		instructions.o 6502.o memory.o cpu.o
TARGET		=		6502.a

include ../../include/Makefile

.PHONY: test

test: functional_test.c $(TARGET)
	$(CC) $(CCFLAGS) $(INCLUDES) -o functional_test $^
	
clean2:
	$(RM) functional_test