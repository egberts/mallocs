#
# Makefile
#

CC ?= cc

SRCTREE  := $(abspath $(CURDIR))
BUILDROOT ?= $(SRCTREE)/build
BUILDROOT := $(abspath $(BUILDROOT))

.DEFAULT_GOAL := all

CONFIG    := $(SRCTREE)/.config
CONFIG_K  := $(SRCTREE)/Kconfig
CONFIG_H  := $(BUILDROOT)/config.h
CONFIG_MK := $(BUILDROOT)/config.mk

TARGET := $(BUILDROOT)/mallocs

MAIN_OBJECT := $(BUILDROOT)/main.o

#
# Build area
#
$(BUILDROOT):
	$(Q)mkdir -p $@

#
# Configuration
#
$(CONFIG_H) $(CONFIG_MK): $(CONFIG_K) $(CONFIG) | $(BUILDROOT)
	$(Q)KCONFIG_CONFIG=$(CONFIG) genconfig $(SRCTREE)/Kconfig \
		--header-path=$(CONFIG_H) \
		--config-out=$(CONFIG_MK)

-include $(CONFIG_MK)

#
# Common compiler flags
#

CPPFLAGS += -I$(BUILDROOT)
CPPFLAGS += -include $(CONFIG_H)

CFLAGS += -O3
CFLAGS += -march=native
CFLAGS += -mtune=native
# CFLAGS += -flto
CFLAGS += -fomit-frame-pointer
CFLAGS += -fno-semantic-interposition
CFLAGS += -fopt-info-inline-optimized-missed
CFLAGS += -Wall
CFLAGS += -Wextra

#
# Subsystems
#

include $(SRCTREE)/buddy/Makefile

#
# Objects
#
$(MAIN_OBJECT): $(SRCTREE)/main.c $(CONFIG_H) | $(BUILDROOT)
	$(Q)$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

#
# Final executable
#

OBJECTS := \
	$(MAIN_OBJECT) \
	$(BUDDY_OBJECTS)

$(TARGET): $(OBJECTS)
	$(Q)$(CC) $(CFLAGS) $^ -o $@

#
# Targets
#

.PHONY: all clean config.h config.mk

all: $(TARGET)

config.h: $(CONFIG_H)

config.mk: $(CONFIG_MK)

clean:
	$(Q)$(RM) $(OBJECTS) $(TARGET) $(CONFIG_H) $(CONFIG_MK)
