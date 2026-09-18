#
# buddy/Makefile
#

BUDDY_OBJECTS :=

ifeq ($(CONFIG_MALLOC_BUDDY_FIBONACCI),y)
BUDDY_OBJECTS += \
	$(BUILDROOT)/buddy/fibonacci/buddy-fibonacci3.o
endif

ifeq ($(CONFIG_MALLOC_BUDDY_BINARY),y)
BUDDY_OBJECTS += \
	$(BUILDROOT)/buddy/binary/buddy-binary.o
endif

ifeq ($(CONFIG_MALLOC_BUDDY_DOUBLE),y)
BUDDY_OBJECTS += \
	$(BUILDROOT)/buddy/double/buddy-double.o
endif

ifeq ($(CONFIG_MALLOC_BUDDY_WEIGHTED),y)
BUDDY_OBJECTS += \
	$(BUILDROOT)/buddy/weighted/buddy-weighted.o
endif

$(BUILDROOT)/buddy/fibonacci/buddy-fibonacci3.o: \
	$(SRCTREE)/buddy/fibonacci/buddy-fibonacci3.c \
	$(CONFIG_H)
	$(Q)$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILDROOT)/buddy/binary/buddy-binary.o: \
	$(SRCTREE)/buddy/binary/buddy-binary.c \
	$(CONFIG_H)
	$(Q)$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILDROOT)/buddy/double/buddy-double.o: \
	$(SRCTREE)/buddy/double/buddy-double.c \
	$(CONFIG_H)
	$(Q)$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILDROOT)/buddy/weighted/buddy-weighted.o: \
	$(SRCTREE)/buddy/weighted/buddy-weighted.c \
	$(CONFIG_H)
	$(Q)$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@
