# buddy/fibonacci/Makefile

CC ?= cc

CONFIG    := $(SRCTREE)/.config
CONFIG_H  := $(BUILDROOT)/config.h
CONFIG_MK := $(BUILDROOT)/config.mk

-include $(CONFIG_MK)

$(info === buddy/fibonacci Makefile ===)
$(info CURDIR    = [$(CURDIR)])
$(info SRCTREE   = [$(SRCTREE)])
$(info BUILDROOT = [$(BUILDROOT)])
$(info CONFIG    = [$(CONFIG)])
$(info CONFIG_H  = [$(CONFIG_H)])
$(info CONFIG_MK = [$(CONFIG_MK)])
$(info CONFIG_MALLOC_BUDDY_FIBONACCI_LAYOUT = [$(CONFIG_MALLOC_BUDDY_FIBONACCI_LAYOUT)])

CPPFLAGS += -I$(BUILDROOT) -include $(CONFIG_H)

obj-y += buddy-fibonacci3.o

OBJECTS_ALL += buddy-fibonacci3.o

OBJECTS_MK := $(BUILDROOT)/buddy/fibonacci/objects.mk

.PHONY: all clean

all: $(obj-y) $(OBJECTS_MK)

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
