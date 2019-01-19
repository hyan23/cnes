# Makefile template for submodules
# written by hyan23
# Date: 2018.05.28

#########################################
##	submodule Makefile example			#
#	OBJECTS		=						#
#	TARGET		=						#
#	<more variables>					#
#	inlcude <template>					#
#	<more rules>						#
#########################################
CC			=		cc
INCLUDES	+=		-I../../include -I../common
CCFLAGS		+=		-Wall
AR			=		ar
ARFLAGS		=		rsv
RM			=		rm -f
TEMP		+=		temp out
OBJECTS		+=		
TARGET		+=		
PREREQUISITES	=	$(OBJECTS:.o=.d)

define clean-cmds
	$(RM) $(TEMP)
	$(RM) $(PREREQUISITES)
	$(RM) $(OBJECTS)
	$(RM) $(TARGET)
endef

.PHONY: all clean clean2

all: $(TARGET)

-include $(PREREQUISITES)

$(PREREQUISITES): %.d: %.c
	$(CC) -MM $(INCLUDES) -o $@ $<
	
$(OBJECTS):
	echo $(CC) $(CCFLAGS) -o $@ -c $<
	$(CC) $(CCFLAGS) $(INCLUDES) -o $@ -c $<

$(TARGET): $(OBJECTS)
	echo $(AR) $(ARFLAGS) $@
	$(AR) $(ARFLAGS) $@ $^ > /dev/null 2>&1

clean: clean2
	$(clean-cmds)