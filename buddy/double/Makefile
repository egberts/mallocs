# buddy/double/Makefile

CC ?= cc
CPPFLAGS += -I../.. -include config.h

CONFIG_MK := ../../config.mk
-include $(CONFIG_MK)

obj-y += buddy-double.o
OBJECTS_ALL += buddy-double.o

.PHONY: all clean

all: $(obj-y)

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJECTS_ALL)
