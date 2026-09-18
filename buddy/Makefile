# buddy/Makefile

SUBDIRS :=

CONFIG    := $(SRCTREE)/.config
CONFIG_H  := $(BUILDROOT)/config.h
CONFIG_MK := $(BUILDROOT)/config.mk
OBJECTS_MK := $(BUILDROOT)/buddy/objects.mk

.DEFAULT_GOAL := all

-include $(CONFIG_MK)

$(info == buddy Makefile == )
$(info CURDIR    = [$(CURDIR)])
$(info SRCTREE   = [$(SRCTREE)])
$(info BUILDROOT = [$(BUILDROOT)])
$(info CONFIG    = [$(CONFIG)])
$(info CONFIG_H  = [$(CONFIG_H)])
$(info CONFIG_MK = [$(CONFIG_MK)])
$(info CONFIG_MALLOC_BUDDY_FIBONACCI_LAYOUT = [$(CONFIG_MALLOC_BUDDY_FIBONACCI_LAYOUT)])

ifneq ($(CONFIG_MALLOC_BUDDY_FIBONACCI),)
SUBDIRS += fibonacci
endif

ifneq ($(CONFIG_MALLOC_BUDDY_BINARY),)
SUBDIRS += binary
endif

ifneq ($(CONFIG_MALLOC_BUDDY_DOUBLE),)
SUBDIRS += double
endif

ifneq ($(CONFIG_MALLOC_BUDDY_WEIGHTED),)
SUBDIRS += weighted
endif

.PHONY: all clean $(SUBDIRS)

all: $(SUBDIRS) $(OBJECTS_MK)

$(SUBDIRS):
	$(Q)$(MAKE) -C $(SRCTREE)/buddy/$@ \
		SRCTREE=$(SRCTREE) \
		BUILDROOT=$(BUILDROOT) \
		V=$(V)

$(OBJECTS_MK): $(SUBDIRS)
	@mkdir -p $(dir $@)
	@printf '%s\n' 'BUDDY_OBJECTS := \' > $@
	@for dir in $(SUBDIRS); do \
		case "$$dir" in \
			fibonacci) \
				printf '%s\n' '$$(BUDDY_FIBONACCI_OBJECTS)' >> $@ ;; \
			binary) \
				printf '%s\n' '$$(BUDDY_BINARY_OBJECTS)' >> $@ ;; \
			double) \
				printf '%s\n' '$$(BUDDY_DOUBLE_OBJECTS)' >> $@ ;; \
			weighted) \
				printf '%s\n' '$$(BUDDY_WEIGHTED_OBJECTS)' >> $@ ;; \
		esac; \
	done

clean:
	$(Q)$(MAKE) -C $(SRCTREE)/buddy/fibonacci \
		SRCTREE=$(SRCTREE) \
		BUILDROOT=$(BUILDROOT) \
		V=$(V) clean
	$(Q)$(MAKE) -C $(SRCTREE)/buddy/binary \
		SRCTREE=$(SRCTREE) \
		BUILDROOT=$(BUILDROOT) \
		V=$(V) clean
	$(Q)$(MAKE) -C $(SRCTREE)/buddy/double \
		SRCTREE=$(SRCTREE) \
		BUILDROOT=$(BUILDROOT) \
		V=$(V) clean
	$(Q)$(MAKE) -C $(SRCTREE)/buddy/weighted \
		SRCTREE=$(SRCTREE) \
		BUILDROOT=$(BUILDROOT) \
		V=$(V) clean
	$(RM) $(OBJECTS_MK)
