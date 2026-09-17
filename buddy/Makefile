SUBDIRS :=
BUILDROOT := $(CURDIR)

-include $(SRCTREE)/config.mk
$(info == buddy Makefile == )
$(info CURDIR    = [$(CURDIR)])
$(info SRCTREE   = [$(SRCTREE)])
$(info BUILDROOT = [$(BUILDROOT)])
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

.PHONE_TARGET: fibonacci binary double weighted

.PHONY: all clean $(SUBDIRS)

all: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $(SRCTREE)/buddy/$@ \
		SRCTREE=$(SRCTREE) \
		BUILDROOT=$(BUILDROOT) \
		V=$(V)

clean:
	$(MAKE) -C $(SRCTREE)/buddy/fibonacci \
		SRCTREE=$(SRCTREE) \
		BUILDROOT=$(BUILDROOT) \
		V=$(V) clean
	$(MAKE) -C $(SRCTREE)/buddy/binary \
		SRCTREE=$(SRCTREE) \
		BUILDROOT=$(BUILDROOT) \
		V=$(V) clean
	$(MAKE) -C $(SRCTREE)/buddy/double \
		SRCTREE=$(SRCTREE) \
		BUILDROOT=$(BUILDROOT) \
		V=$(V) clean
	$(MAKE) -C $(SRCTREE)/buddy/weighted \
		SRCTREE=$(SRCTREE) \
		BUILDROOT=$(BUILDROOT) \
		V=$(V) clean
