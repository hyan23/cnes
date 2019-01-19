# Makefile for cnes
# written by hyan23
# Date: 2017.08.01

RM			=		rm -f
TEMP		=		temp out
MODULES		=		src/nes/nes.a		\
					src/mmc/mmc.a		\
					src/2A03/2A03.a		\
					src/2C02/2C02.a		\
					src/6502/6502.a		\
					src/input/input.a	\
					src/rom/rom.a		\
					src/ini/ini.a		\
					src/common/common.a
TARGET		=		cnes
DIRS		=		$(dir $(wildcard $(MODULES)))

.PHONY: all clean $(MODULES) $(DIRS)

all: $(TARGET)

$(TARGET): src/cnes.c $(MODULES) -lm -lSDL
	@echo build $@
	@cc -Wall -o $@ -Iinclude -Isrc/common $^

$(MODULES): %.a: %.c
	@make -s -C $(dir $@)
	
$(DIRS): %: 
	@make -s -C $@ clean
	
clean: $(DIRS)
	@$(RM) $(TEMP)
	@$(RM) cnes