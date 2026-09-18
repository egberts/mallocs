#
# Makefile
#

CC ?= cc

SRCTREE  := $(abspath $(CURDIR))
BUILDROOT ?= $(SRCTREE)/build
BUILDROOT := $(abspath $(BUILDROOT))

.DEFAULT_GOAL := all

TOOLS := $(SRCTREE)/tools

CONFIG    := $(SRCTREE)/.config
CONFIG_H  := $(BUILDROOT)/config.h
CONFIG_MK := $(BUILDROOT)/config.mk

BUDDY_OBJECTS_MK := $(BUILDROOT)/buddy/objects.mk

TARGET := $(BUILDROOT)/mallocs

#
# Configuration
#

$(CONFIG_H) $(CONFIG_MK): $(SRCTREE)/Kconfig $(CONFIG)
	$(Q)KCONFIG_CONFIG=$(CONFIG) genconfig $(SRCTREE)/Kconfig \
		--header-path=$(CONFIG_H) \
		--config-out=$(CONFIG_MK)

-include $(CONFIG_MK)

#
# Buddy allocator build interface
#

$(BUDDY_OBJECTS_MK): $(CONFIG_MK)
	$(Q)$(MAKE) -C $(SRCTREE)/buddy \
		SRCTREE=$(SRCTREE) \
		BUILDROOT=$(BUILDROOT) \
		V=$(V)

-include $(BUDDY_OBJECTS_MK)

#
# Main program
#

MAIN_OBJECT := $(BUILDROOT)/main.o

CFLAGS += -I$(BUILDROOT)
CFLAGS += -include $(CONFIG_H)

$(MAIN_OBJECT): $(SRCTREE)/main.c $(CONFIG_H)
	$(Q)$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

#
# Final executable
#

$(TARGET): $(CONFIG_H) $(CONFIG_MK) $(BUDDY_OBJECTS_MK) $(MAIN_OBJECT)
	$(Q)$(CC) $(CFLAGS) $(MAIN_OBJECT) $(BUDDY_OBJECTS) -o $@

#
# User-facing targets
#

.PHONY: all clean config.h config.mk buddy

all: $(TARGET)

config.h: $(CONFIG_H)

config.mk: $(CONFIG_MK)

buddy: $(BUDDY_OBJECTS_MK)

clean:
	$(Q)$(RM) $(MAIN_OBJECT) $(TARGET) $(CONFIG_H) $(CONFIG_MK) $(BUDDY_OBJECTS_MK)
	$(Q)$(MAKE) -C $(SRCTREE)/buddy \
		SRCTREE=$(SRCTREE) \
		BUILDROOT=$(BUILDROOT) \
		V=$(V) clean
