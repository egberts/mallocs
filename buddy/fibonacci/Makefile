# buddy/fibonacci/Makefile

$(info === buddy/fibanocci Makefile === )
$(info CURDIR    = [$(CURDIR)])
$(info SRCTREE   = [$(SRCTREE)])
$(info BUILDROOT = [$(BUILDROOT)])
$(info CONFIG_MALLOC_BUDDY_FIBONACCI_LAYOUT = [$(CONFIG_MALLOC_BUDDY_FIBONACCI_LAYOUT)])

CC ?= cc

SRCTREE   ?= $(abspath ../..)
BUILDROOT ?= $(SRCTREE)
CONFIG_MK := $(BUILDROOT)/config.mk
include $(CONFIG_MK)


CPPFLAGS += -I$(BUILDROOT) -include config.h

obj-y += buddy-fibonacci3.o

OBJECTS_ALL += buddy-fibonacci3.o

OBJECTS_MK := $(BUILDROOT)/buddy/fibonacci/objects.mk

.PHONY: all clean

all: $(obj-y) $(OBJECTS_MK)

config.mk: $(SRCTREE)/.config
	$(SRCTREE)/tools/genmakeconfig $< > $@

$(OBJECTS_MK): $(obj-y)
	@mkdir -p $(dir $@)
	@printf '%s\n' \
		'BUDDY_FIBONACCI_OBJECTS := \' > $@
	@printf '%s\n' $(addprefix $(BUILDROOT)/buddy/fibonacci/,$(obj-y)) \
		>> $@

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(addprefix $(BUILDROOT)/buddy/fibonacci/,$(OBJECTS_ALL))
	$(RM) $(OBJECTS_MK)
