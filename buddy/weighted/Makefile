# buddy/weighted/Makefile

CC ?= cc
CPPFLAGS += -I../.. -include config.h

CONFIG_MK := ../../config.mk
-include $(CONFIG_MK)

obj-y += buddy-weighted.o
OBJECTS_ALL += buddy-weighted.o

.PHONY: all clean

all: $(obj-y)

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJECTS_ALL)
