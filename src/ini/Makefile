# Makefile for ini
# written by hyan23
# Date: 2018.05.27

OBJECTS		=		pair.o section.o ini.o
TARGET		=		ini.a

.PHONY: test

include ../../include/Makefile

test: test.c $(TARGET) ../common/common.a
	$(CC) $(CCFLAGS) $(INCLUDES) -o $@ $^
	
clean2:
	$(RM) test