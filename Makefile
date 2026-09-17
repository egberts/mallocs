#

CC ?= cc

SRCTREE   ?= $(CURDIR)
BUILDROOT ?= .

.DEFAULT_GOAL := mallocs

TOOLS := $(SRCTREE)/tools

TARGET    := $(BUILDROOT)/mallocs
CONFIG    := $(BUILDROOT)/.config
CONFIG_H  := $(BUILDROOT)/config.h
CONFIG_MK := $(BUILDROOT)/config.mk

CPPFLAGS += -I$(BUILDROOT) -include config.h

CFLAGS := -O3 -march=native -mtune=native \
          -flto -fomit-frame-pointer \
          -fno-semantic-interposition \
          -Wall -Wextra

V ?= 0

ifeq ($(V),1)
Q :=
else
Q := @
endif

# Kconfig-generated Make variables.
-include $(CONFIG_MK)
$(info = top-level Makefile = )
$(info CURDIR    = [$(CURDIR)])
$(info SRCTREE   = [$(SRCTREE)])
$(info BUILDROOT = [$(BUILDROOT)])
$(info CONFIG_MALLOC_BUDDY_FIBONACCI_LAYOUT = [$(CONFIG_MALLOC_BUDDY_FIBONACCI_LAYOUT)])


# Generate config.h from .config.
$(CONFIG_H): $(CONFIG)
	$(Q)mkdir -p $(dir $@)
	$(Q)genconfig $(CONFIG)

# Generate config.mk from .config.
$(CONFIG_MK): $(CONFIG)
	$(Q)mkdir -p $(dir $@)
	$(Q)$(TOOLS)/genmakeconfig $(CONFIG)

# Main program.
MAIN_OBJECT := $(BUILDROOT)/main.o

$(MAIN_OBJECT): $(SRCTREE)/main.c $(CONFIG_H)
	$(Q)mkdir -p $(dir $@)
	$(Q)$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# The buddy subsystem owns its allocator implementations.
.PHONY: buddy

buddy:
	$(Q)$(MAKE) -C $(SRCTREE)/buddy \
		SRCTREE=$(SRCTREE) \
		BUILDROOT=$(BUILDROOT) \
		V=$(V)

# Final executable.
#
# BUDDY_OBJECTS will be supplied by the buddy build interface.
$(TARGET): $(CONFIG_H) $(CONFIG_MK) $(MAIN_OBJECT) buddy
	$(Q)$(CC) $(CFLAGS) $(MAIN_OBJECT) $(BUDDY_OBJECTS) -o $@

.PHONY: all clean

all: $(TARGET)

clean:
	$(Q)$(RM) $(MAIN_OBJECT) $(TARGET) $(CONFIG_H) $(CONFIG_MK)
	$(Q)$(MAKE) -C $(SRCTREE)/buddy \
		SRCTREE=$(SRCTREE) \
		BUILDROOT=$(BUILDROOT) \
		V=$(V) clean
