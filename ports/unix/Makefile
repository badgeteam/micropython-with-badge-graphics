# Select the variant to build for.
VARIANT ?= standard

# If the build directory is not given, make it reflect the variant name.
BUILD ?= build-$(VARIANT)

VARIANT_DIR ?= variants/$(VARIANT)
ifeq ($(wildcard $(VARIANT_DIR)/.),)
$(error Invalid VARIANT specified: $(VARIANT_DIR))
endif

include ../../py/mkenv.mk
-include mpconfigport.mk
include $(VARIANT_DIR)/mpconfigvariant.mk

# use FROZEN_MANIFEST for new projects, others are legacy
FROZEN_MANIFEST ?= variants/manifest.py
FROZEN_DIR =
FROZEN_MPY_DIR =

# This should be configured by the mpconfigvariant.mk
PROG ?= micropython

# qstr definitions (must come before including py.mk)
QSTR_DEFS = qstrdefsport.h
QSTR_GLOBAL_DEPENDENCIES = $(VARIANT_DIR)/mpconfigvariant.h

# OS name, for simple autoconfig
UNAME_S := $(shell uname -s)

# include py core make definitions
include $(TOP)/py/py.mk

GIT_SUBMODULES += lib/axtls lib/berkeley-db-1.xx lib/libffi

INC +=  -I.
INC +=  -I$(TOP)
INC += -I$(BUILD)

# compiler settings
CWARN = -Wall
CWARN += -Wextra -Wno-unused-parameter -Wpointer-arith -Wdouble-promotion -Wfloat-conversion
CFLAGS += $(INC) $(CWARN) -std=gnu99 -DUNIX $(CFLAGS_MOD) $(COPT) -I$(VARIANT_DIR) $(CFLAGS_EXTRA)

# Debugging/Optimization
ifdef DEBUG
COPT ?= -O0
else
COPT ?= -Os
COPT += -DNDEBUG
endif

# Remove unused sections.
COPT += -fdata-sections -ffunction-sections

# Always enable symbols -- They're occasionally useful, and don't make it into the
# final .bin/.hex/.dfu so the extra size doesn't matter.
CFLAGS += -g

ifndef DEBUG
# _FORTIFY_SOURCE is a feature in gcc/glibc which is intended to provide extra
# security for detecting buffer overflows. Some distros (Ubuntu at the very least)
# have it enabled by default.
#
# gcc already optimizes some printf calls to call puts and/or putchar. When
# _FORTIFY_SOURCE is enabled and compiling with -O1 or greater, then some
# printf calls will also be optimized to call __printf_chk (in glibc). Any
# printfs which get redirected to __printf_chk are then no longer synchronized
# with printfs that go through mp_printf.
#
# In MicroPython, we don't want to use the runtime library's printf but rather
# go through mp_printf, so that stdout is properly tied into streams, etc.
# This means that we either need to turn off _FORTIFY_SOURCE or provide our
# own implementation of __printf_chk. We've chosen to turn off _FORTIFY_SOURCE.
# It should also be noted that the use of printf in MicroPython is typically
# quite limited anyways (primarily for debug and some error reporting, etc
# in the unix version).
#
# Information about _FORTIFY_SOURCE seems to be rather scarce. The best I could
# find was this: https://securityblog.redhat.com/2014/03/26/fortify-and-you/
# Original patchset was introduced by
# https://gcc.gnu.org/ml/gcc-patches/2004-09/msg02055.html .
#
# Turning off _FORTIFY_SOURCE is only required when compiling with -O1 or greater
CFLAGS += -U _FORTIFY_SOURCE
endif

# On OSX, 'gcc' is a symlink to clang unless a real gcc is installed.
# The unix port of MicroPython on OSX must be compiled with clang,
# while cross-compile ports require gcc, so we test here for OSX and
# if necessary override the value of 'CC' set in py/mkenv.mk
ifeq ($(UNAME_S),Darwin)
ifeq ($(MICROPY_FORCE_32BIT),1)
CC = clang -m32
else
CC = clang
endif
# Use clang syntax for map file
LDFLAGS_ARCH = -Wl,-map,$@.map -Wl,-dead_strip
else
# Use gcc syntax for map file
LDFLAGS_ARCH = -Wl,-Map=$@.map,--cref -Wl,--gc-sections
endif
LDFLAGS += $(LDFLAGS_MOD) $(LDFLAGS_ARCH) -lm $(LDFLAGS_EXTRA)

# Flags to link with pthread library
LIBPTHREAD = -lpthread

ifeq ($(MICROPY_FORCE_32BIT),1)
# Note: you may need to install i386 versions of dependency packages,
# starting with linux-libc-dev:i386
ifeq ($(MICROPY_PY_FFI),1)
ifeq ($(UNAME_S),Linux)
CFLAGS_MOD += -I/usr/include/i686-linux-gnu
endif
endif
endif

ifeq ($(MICROPY_USE_READLINE),1)
INC +=  -I$(TOP)/lib/mp-readline
CFLAGS_MOD += -DMICROPY_USE_READLINE=1
LIB_SRC_C_EXTRA += mp-readline/readline.c
endif
ifeq ($(MICROPY_PY_TERMIOS),1)
CFLAGS_MOD += -DMICROPY_PY_TERMIOS=1
SRC_MOD += modtermios.c
endif
ifeq ($(MICROPY_PY_SOCKET),1)
CFLAGS_MOD += -DMICROPY_PY_SOCKET=1
SRC_MOD += modusocket.c
endif
ifeq ($(MICROPY_PY_THREAD),1)
CFLAGS_MOD += -DMICROPY_PY_THREAD=1 -DMICROPY_PY_THREAD_GIL=0
LDFLAGS_MOD += $(LIBPTHREAD)
endif

# If the variant enables it, enable modbluetooth.
ifeq ($(MICROPY_PY_BLUETOOTH),1)

HAVE_LIBUSB := $(shell (which pkg-config > /dev/null && pkg-config --exists libusb-1.0) 2>/dev/null && echo '1')

# Only one stack can be enabled.
ifeq ($(MICROPY_BLUETOOTH_NIMBLE),1)
ifeq ($(MICROPY_BLUETOOTH_BTSTACK),1)
$(error Cannot enable both NimBLE and BTstack at the same time)
endif
endif

# Default to btstack, but a variant (or make command line) can set NimBLE
# explicitly (which is always via H4 UART).
ifneq ($(MICROPY_BLUETOOTH_NIMBLE),1)
ifneq ($(MICROPY_BLUETOOTH_BTSTACK),1)
MICROPY_BLUETOOTH_BTSTACK ?= 1
endif
endif

CFLAGS_MOD += -DMICROPY_PY_BLUETOOTH=1
CFLAGS_MOD += -DMICROPY_PY_BLUETOOTH_ENABLE_CENTRAL_MODE=1
# Runs in a thread, cannot make calls into the VM.
CFLAGS_MOD += -DMICROPY_PY_BLUETOOTH_GATTS_ON_READ_CALLBACK=0

ifeq ($(MICROPY_BLUETOOTH_BTSTACK),1)

# Figure out which BTstack transport to use.
ifeq ($(MICROPY_BLUETOOTH_BTSTACK_H4),1)
ifeq ($(MICROPY_BLUETOOTH_BTSTACK_USB),1)
$(error Cannot enable BTstack support for USB and H4 UART at the same time)
endif
else
ifeq ($(HAVE_LIBUSB),1)
# Default to btstack-over-usb.
MICROPY_BLUETOOTH_BTSTACK_USB ?= 1
else
# Fallback to HCI controller via a H4 UART (e.g. Zephyr on nRF) over a /dev/tty serial port.
MICROPY_BLUETOOTH_BTSTACK_H4 ?= 1
endif
endif

# BTstack is enabled.
GIT_SUBMODULES += lib/btstack
include $(TOP)/extmod/btstack/btstack.mk

else

# NimBLE is enabled.
GIT_SUBMODULES += lib/mynewt-nimble
include $(TOP)/extmod/nimble/nimble.mk

endif

endif

ifeq ($(MICROPY_PY_FFI),1)

ifeq ($(MICROPY_STANDALONE),1)
LIBFFI_CFLAGS_MOD := -I$(shell ls -1d $(BUILD)/lib/libffi/out/lib/libffi-*/include)
 ifeq ($(MICROPY_FORCE_32BIT),1)
  LIBFFI_LDFLAGS_MOD = $(BUILD)/lib/libffi/out/lib32/libffi.a
 else
  LIBFFI_LDFLAGS_MOD = $(BUILD)/lib/libffi/out/lib/libffi.a
 endif
else
LIBFFI_CFLAGS_MOD := $(shell pkg-config --cflags libffi)
LIBFFI_LDFLAGS_MOD := $(shell pkg-config --libs libffi)
endif

ifeq ($(UNAME_S),Linux)
LIBFFI_LDFLAGS_MOD += -ldl
endif

CFLAGS_MOD += $(LIBFFI_CFLAGS_MOD) -DMICROPY_PY_FFI=1
LDFLAGS_MOD += $(LIBFFI_LDFLAGS_MOD)
SRC_MOD += modffi.c
endif

ifeq ($(MICROPY_PY_JNI),1)
# Path for 64-bit OpenJDK, should be adjusted for other JDKs
CFLAGS_MOD += -I/usr/lib/jvm/java-7-openjdk-amd64/include -DMICROPY_PY_JNI=1
SRC_MOD += modjni.c
endif

CFLAGS += -DCONFIG_DRIVER_FRAMEBUFFER_ENABLE
CFLAGS += -D_REENTRANT
LDFLAGS += -pthread -lSDL2main -lSDL2 -lSDL2_image -lm

INC += \
	-Idisplay/include \
	-Idisplay/png \
	-I/usr/include/SDL2 \

SRC_C += \
	display/png/adler32.c \
	display/png/crc32.c \
	display/png/deflate_reader.c \
	display/png/mem_reader.c \
	display/png/png_reader.c \
	display/sdl.c \
	display/fonts/font_Exo2-BlackItalic_12pt7b.c \
	display/fonts/font_Exo2-BlackItalic_13pt7b.c \
	display/fonts/font_Exo2-BlackItalic_16pt7b.c \
	display/fonts/font_Exo2-BlackItalic_18pt7b.c \
	display/fonts/font_Exo2-BlackItalic_20pt7b.c \
	display/fonts/font_Exo2-BlackItalic_22pt7b.c \
	display/fonts/font_Exo2-BlackItalic_24pt7b.c \
	display/fonts/font_Exo2-BlackItalic_36pt7b.c \
	display/fonts/font_Exo2-BlackItalic_42pt7b.c \
	display/fonts/font_Exo2-BlackItalic_6pt7b.c \
	display/fonts/font_Exo2-BlackItalic_8pt7b.c \
	display/fonts/font_Exo2-BlackItalic_9pt7b.c \
	display/fonts/font_Exo2-Black_12pt7b.c \
	display/fonts/font_Exo2-Black_13pt7b.c \
	display/fonts/font_Exo2-Black_16pt7b.c \
	display/fonts/font_Exo2-Black_18pt7b.c \
	display/fonts/font_Exo2-Black_20pt7b.c \
	display/fonts/font_Exo2-Black_22pt7b.c \
	display/fonts/font_Exo2-Black_24pt7b.c \
	display/fonts/font_Exo2-Black_36pt7b.c \
	display/fonts/font_Exo2-Black_42pt7b.c \
	display/fonts/font_Exo2-Black_6pt7b.c \
	display/fonts/font_Exo2-Black_8pt7b.c \
	display/fonts/font_Exo2-Black_9pt7b.c \
	display/fonts/font_Exo2-BoldItalic_12pt7b.c \
	display/fonts/font_Exo2-BoldItalic_13pt7b.c \
	display/fonts/font_Exo2-BoldItalic_16pt7b.c \
	display/fonts/font_Exo2-BoldItalic_18pt7b.c \
	display/fonts/font_Exo2-BoldItalic_20pt7b.c \
	display/fonts/font_Exo2-BoldItalic_22pt7b.c \
	display/fonts/font_Exo2-BoldItalic_24pt7b.c \
	display/fonts/font_Exo2-BoldItalic_36pt7b.c \
	display/fonts/font_Exo2-BoldItalic_42pt7b.c \
	display/fonts/font_Exo2-BoldItalic_6pt7b.c \
	display/fonts/font_Exo2-BoldItalic_8pt7b.c \
	display/fonts/font_Exo2-BoldItalic_9pt7b.c \
	display/fonts/font_Exo2-Bold_12pt7b.c \
	display/fonts/font_Exo2-Bold_13pt7b.c \
	display/fonts/font_Exo2-Bold_16pt7b.c \
	display/fonts/font_Exo2-Bold_18pt7b.c \
	display/fonts/font_Exo2-Bold_20pt7b.c \
	display/fonts/font_Exo2-Bold_22pt7b.c \
	display/fonts/font_Exo2-Bold_24pt7b.c \
	display/fonts/font_Exo2-Bold_36pt7b.c \
	display/fonts/font_Exo2-Bold_42pt7b.c \
	display/fonts/font_Exo2-Bold_6pt7b.c \
	display/fonts/font_Exo2-Bold_8pt7b.c \
	display/fonts/font_Exo2-Bold_9pt7b.c \
	display/fonts/font_Exo2-ExtraBoldItalic_12pt7b.c \
	display/fonts/font_Exo2-ExtraBoldItalic_13pt7b.c \
	display/fonts/font_Exo2-ExtraBoldItalic_16pt7b.c \
	display/fonts/font_Exo2-ExtraBoldItalic_18pt7b.c \
	display/fonts/font_Exo2-ExtraBoldItalic_20pt7b.c \
	display/fonts/font_Exo2-ExtraBoldItalic_22pt7b.c \
	display/fonts/font_Exo2-ExtraBoldItalic_24pt7b.c \
	display/fonts/font_Exo2-ExtraBoldItalic_36pt7b.c \
	display/fonts/font_Exo2-ExtraBoldItalic_42pt7b.c \
	display/fonts/font_Exo2-ExtraBoldItalic_6pt7b.c \
	display/fonts/font_Exo2-ExtraBoldItalic_8pt7b.c \
	display/fonts/font_Exo2-ExtraBoldItalic_9pt7b.c \
	display/fonts/font_Exo2-ExtraBold_12pt7b.c \
	display/fonts/font_Exo2-ExtraBold_13pt7b.c \
	display/fonts/font_Exo2-ExtraBold_16pt7b.c \
	display/fonts/font_Exo2-ExtraBold_18pt7b.c \
	display/fonts/font_Exo2-ExtraBold_20pt7b.c \
	display/fonts/font_Exo2-ExtraBold_22pt7b.c \
	display/fonts/font_Exo2-ExtraBold_24pt7b.c \
	display/fonts/font_Exo2-ExtraBold_36pt7b.c \
	display/fonts/font_Exo2-ExtraBold_42pt7b.c \
	display/fonts/font_Exo2-ExtraBold_6pt7b.c \
	display/fonts/font_Exo2-ExtraBold_8pt7b.c \
	display/fonts/font_Exo2-ExtraBold_9pt7b.c \
	display/fonts/font_Exo2-ExtraLightItalic_12pt7b.c \
	display/fonts/font_Exo2-ExtraLightItalic_13pt7b.c \
	display/fonts/font_Exo2-ExtraLightItalic_16pt7b.c \
	display/fonts/font_Exo2-ExtraLightItalic_18pt7b.c \
	display/fonts/font_Exo2-ExtraLightItalic_20pt7b.c \
	display/fonts/font_Exo2-ExtraLightItalic_22pt7b.c \
	display/fonts/font_Exo2-ExtraLightItalic_24pt7b.c \
	display/fonts/font_Exo2-ExtraLightItalic_36pt7b.c \
	display/fonts/font_Exo2-ExtraLightItalic_42pt7b.c \
	display/fonts/font_Exo2-ExtraLightItalic_6pt7b.c \
	display/fonts/font_Exo2-ExtraLightItalic_8pt7b.c \
	display/fonts/font_Exo2-ExtraLightItalic_9pt7b.c \
	display/fonts/font_Exo2-ExtraLight_12pt7b.c \
	display/fonts/font_Exo2-ExtraLight_13pt7b.c \
	display/fonts/font_Exo2-ExtraLight_16pt7b.c \
	display/fonts/font_Exo2-ExtraLight_18pt7b.c \
	display/fonts/font_Exo2-ExtraLight_20pt7b.c \
	display/fonts/font_Exo2-ExtraLight_6pt7b.c \
	display/fonts/font_Exo2-ExtraLight_22pt7b.c \
	display/fonts/font_Exo2-ExtraLight_24pt7b.c \
	display/fonts/font_Exo2-ExtraLight_36pt7b.c \
	display/fonts/font_Exo2-ExtraLight_42pt7b.c \
	display/fonts/font_Exo2-ExtraLight_8pt7b.c \
	display/fonts/font_Exo2-ExtraLight_9pt7b.c \
	display/fonts/font_Exo2-Italic_12pt7b.c \
	display/fonts/font_Exo2-Italic_13pt7b.c \
	display/fonts/font_Exo2-Italic_16pt7b.c \
	display/fonts/font_Exo2-Italic_18pt7b.c \
	display/fonts/font_Exo2-Italic_20pt7b.c \
	display/fonts/font_Exo2-Italic_22pt7b.c \
	display/fonts/font_Exo2-Italic_24pt7b.c \
	display/fonts/font_Exo2-Italic_36pt7b.c \
	display/fonts/font_Exo2-Italic_42pt7b.c \
	display/fonts/font_Exo2-Italic_6pt7b.c \
	display/fonts/font_Exo2-Italic_8pt7b.c \
	display/fonts/font_Exo2-Italic_9pt7b.c \
	display/fonts/font_Exo2-LightItalic_12pt7b.c \
	display/fonts/font_Exo2-LightItalic_13pt7b.c \
	display/fonts/font_Exo2-LightItalic_16pt7b.c \
	display/fonts/font_Exo2-LightItalic_18pt7b.c \
	display/fonts/font_Exo2-LightItalic_20pt7b.c \
	display/fonts/font_Exo2-LightItalic_22pt7b.c \
	display/fonts/font_Exo2-LightItalic_24pt7b.c \
	display/fonts/font_Exo2-LightItalic_36pt7b.c \
	display/fonts/font_Exo2-LightItalic_42pt7b.c \
	display/fonts/font_Exo2-LightItalic_6pt7b.c \
	display/fonts/font_Exo2-LightItalic_8pt7b.c \
	display/fonts/font_Exo2-LightItalic_9pt7b.c \
	display/fonts/font_Exo2-Light_12pt7b.c \
	display/fonts/font_Exo2-Light_13pt7b.c \
	display/fonts/font_Exo2-Light_16pt7b.c \
	display/fonts/font_Exo2-Light_18pt7b.c \
	display/fonts/font_Exo2-Light_20pt7b.c \
	display/fonts/font_Exo2-Light_22pt7b.c \
	display/fonts/font_Exo2-Light_24pt7b.c \
	display/fonts/font_Exo2-Light_36pt7b.c \
	display/fonts/font_Exo2-Light_42pt7b.c \
	display/fonts/font_Exo2-Light_6pt7b.c \
	display/fonts/font_Exo2-Light_8pt7b.c \
	display/fonts/font_Exo2-Light_9pt7b.c \
	display/fonts/font_Exo2-MediumItalic_12pt7b.c \
	display/fonts/font_Exo2-MediumItalic_13pt7b.c \
	display/fonts/font_Exo2-MediumItalic_16pt7b.c \
	display/fonts/font_Exo2-MediumItalic_18pt7b.c \
	display/fonts/font_Exo2-MediumItalic_20pt7b.c \
	display/fonts/font_Exo2-MediumItalic_22pt7b.c \
	display/fonts/font_Exo2-MediumItalic_24pt7b.c \
	display/fonts/font_Exo2-MediumItalic_36pt7b.c \
	display/fonts/font_Exo2-MediumItalic_42pt7b.c \
	display/fonts/font_Exo2-MediumItalic_6pt7b.c \
	display/fonts/font_Exo2-MediumItalic_8pt7b.c \
	display/fonts/font_Exo2-MediumItalic_9pt7b.c \
	display/fonts/font_Exo2-Medium_12pt7b.c \
	display/fonts/font_Exo2-Medium_13pt7b.c \
	display/fonts/font_Exo2-Medium_16pt7b.c \
	display/fonts/font_Exo2-Medium_18pt7b.c \
	display/fonts/font_Exo2-Medium_20pt7b.c \
	display/fonts/font_Exo2-Medium_22pt7b.c \
	display/fonts/font_Exo2-Medium_24pt7b.c \
	display/fonts/font_Exo2-Medium_36pt7b.c \
	display/fonts/font_Exo2-Medium_42pt7b.c \
	display/fonts/font_Exo2-Medium_6pt7b.c \
	display/fonts/font_Exo2-Medium_8pt7b.c \
	display/fonts/font_Exo2-Medium_9pt7b.c \
	display/fonts/font_Exo2-Regular_12pt7b.c \
	display/fonts/font_Exo2-Regular_13pt7b.c \
	display/fonts/font_Exo2-Regular_16pt7b.c \
	display/fonts/font_Exo2-Regular_18pt7b.c \
	display/fonts/font_Exo2-Regular_20pt7b.c \
	display/fonts/font_Exo2-Regular_22pt7b.c \
	display/fonts/font_Exo2-Regular_24pt7b.c \
	display/fonts/font_Exo2-Regular_36pt7b.c \
	display/fonts/font_Exo2-Regular_42pt7b.c \
	display/fonts/font_Exo2-Regular_6pt7b.c \
	display/fonts/font_Exo2-Regular_8pt7b.c \
	display/fonts/font_Exo2-Regular_9pt7b.c \
	display/fonts/font_Exo2-SemiBoldItalic_12pt7b.c \
	display/fonts/font_Exo2-SemiBoldItalic_13pt7b.c \
	display/fonts/font_Exo2-SemiBoldItalic_16pt7b.c \
	display/fonts/font_Exo2-SemiBoldItalic_18pt7b.c \
	display/fonts/font_Exo2-SemiBoldItalic_20pt7b.c \
	display/fonts/font_Exo2-SemiBoldItalic_22pt7b.c \
	display/fonts/font_Exo2-SemiBoldItalic_24pt7b.c \
	display/fonts/font_Exo2-SemiBoldItalic_36pt7b.c \
	display/fonts/font_Exo2-SemiBoldItalic_42pt7b.c \
	display/fonts/font_Exo2-SemiBoldItalic_6pt7b.c \
	display/fonts/font_Exo2-SemiBoldItalic_8pt7b.c \
	display/fonts/font_Exo2-SemiBoldItalic_9pt7b.c \
	display/fonts/font_Exo2-SemiBold_12pt7b.c \
	display/fonts/font_ipane7x5.c \
	display/fonts/font_Exo2-SemiBold_13pt7b.c \
	display/fonts/font_Exo2-SemiBold_16pt7b.c \
	display/fonts/font_Exo2-SemiBold_18pt7b.c \
	display/fonts/font_Exo2-SemiBold_20pt7b.c \
	display/fonts/font_Exo2-SemiBold_22pt7b.c \
	display/fonts/font_Exo2-SemiBold_24pt7b.c \
	display/fonts/font_Exo2-SemiBold_36pt7b.c \
	display/fonts/font_Exo2-SemiBold_42pt7b.c \
	display/fonts/font_Exo2-SemiBold_6pt7b.c \
	display/fonts/font_Exo2-SemiBold_8pt7b.c \
	display/fonts/font_Exo2-SemiBold_9pt7b.c \
	display/fonts/font_Exo2-ThinItalic_12pt7b.c \
	display/fonts/font_Exo2-ThinItalic_13pt7b.c \
	display/fonts/font_Exo2-ThinItalic_16pt7b.c \
	display/fonts/font_Exo2-ThinItalic_18pt7b.c \
	display/fonts/font_Exo2-ThinItalic_20pt7b.c \
	display/fonts/font_Exo2-ThinItalic_22pt7b.c \
	display/fonts/font_Exo2-ThinItalic_24pt7b.c \
	display/fonts/font_Exo2-ThinItalic_36pt7b.c \
	display/fonts/font_Exo2-ThinItalic_42pt7b.c \
	display/fonts/font_Exo2-ThinItalic_6pt7b.c \
	display/fonts/font_Exo2-ThinItalic_8pt7b.c \
	display/fonts/font_Exo2-ThinItalic_9pt7b.c \
	display/fonts/font_Exo2-Thin_12pt7b.c \
	display/fonts/font_Exo2-Thin_13pt7b.c \
	display/fonts/font_Exo2-Thin_16pt7b.c \
	display/fonts/font_Exo2-Thin_18pt7b.c \
	display/fonts/font_Exo2-Thin_20pt7b.c \
	display/fonts/font_Exo2-Thin_22pt7b.c \
	display/fonts/font_Exo2-Thin_24pt7b.c \
	display/fonts/font_Exo2-Thin_36pt7b.c \
	display/fonts/font_Exo2-Thin_42pt7b.c \
	display/fonts/font_Exo2-Thin_6pt7b.c \
	display/fonts/font_Exo2-Thin_8pt7b.c \
	display/fonts/font_Exo2-Thin_9pt7b.c \
	display/fonts/font_dejavusans_12pt7b.c \
	display/fonts/font_dejavusans_13pt7b.c \
	display/fonts/font_dejavusans_16pt7b.c \
	display/fonts/font_dejavusans_18pt7b.c \
	display/fonts/font_dejavusans_20pt7b.c \
	display/fonts/font_dejavusans_22pt7b.c \
	display/fonts/font_dejavusans_24pt7b.c \
	display/fonts/font_dejavusans_36pt7b.c \
	display/fonts/font_dejavusans_42pt7b.c \
	display/fonts/font_dejavusans_6pt7b.c \
	display/fonts/font_dejavusans_8pt7b.c \
	display/fonts/font_dejavusans_9pt7b.c \
	display/fonts/font_fairlight_12pt7b.c \
	display/fonts/font_fairlight_13pt7b.c \
	display/fonts/font_fairlight_16pt7b.c \
	display/fonts/font_fairlight_18pt7b.c \
	display/fonts/font_fairlight_20pt7b.c \
	display/fonts/font_fairlight_22pt7b.c \
	display/fonts/font_fairlight_24pt7b.c \
	display/fonts/font_fairlight_36pt7b.c \
	display/fonts/font_fairlight_42pt7b.c \
	display/fonts/font_fairlight_6pt7b.c \
	display/fonts/font_fairlight_8pt7b.c \
	display/fonts/font_fairlight_9pt7b.c \
	display/fonts/font_ocra_12pt7b.c \
	display/fonts/font_ocra_13pt7b.c \
	display/fonts/font_ocra_16pt7b.c \
	display/fonts/font_ocra_18pt7b.c \
	display/fonts/font_ocra_20pt7b.c \
	display/fonts/font_ocra_22pt7b.c \
	display/fonts/font_ocra_24pt7b.c \
	display/fonts/font_ocra_36pt7b.c \
	display/fonts/font_ocra_42pt7b.c \
	display/fonts/font_ocra_6pt7b.c \
	display/fonts/font_ocra_8pt7b.c \
	display/fonts/font_ocra_9pt7b.c \
	display/fonts/font_org_01_12pt7b.c \
	display/fonts/font_org_01_13pt7b.c \
	display/fonts/font_org_01_16pt7b.c \
	display/fonts/font_org_01_18pt7b.c \
	display/fonts/font_org_01_20pt7b.c \
	display/fonts/font_org_01_22pt7b.c \
	display/fonts/font_org_01_24pt7b.c \
	display/fonts/font_org_01_36pt7b.c \
	display/fonts/font_org_01_42pt7b.c \
	display/fonts/font_org_01_6pt7b.c \
	display/fonts/font_org_01_8pt7b.c \
	display/fonts/font_org_01_9pt7b.c \
	display/fonts/font_permanentmarker_12pt7b.c \
	display/fonts/font_permanentmarker_13pt7b.c \
	display/fonts/font_permanentmarker_16pt7b.c \
	display/fonts/font_permanentmarker_18pt7b.c \
	display/fonts/font_permanentmarker_20pt7b.c \
	display/fonts/font_permanentmarker_22pt7b.c \
	display/fonts/font_permanentmarker_24pt7b.c \
	display/fonts/font_permanentmarker_36pt7b.c \
	display/fonts/font_permanentmarker_42pt7b.c \
	display/fonts/font_permanentmarker_6pt7b.c \
	display/fonts/font_permanentmarker_8pt7b.c \
	display/fonts/font_permanentmarker_9pt7b.c \
	display/fonts/font_roboto-BlackItalic_12pt7b.c \
	display/fonts/font_roboto-BlackItalic_13pt7b.c \
	display/fonts/font_roboto-BlackItalic_16pt7b.c \
	display/fonts/font_roboto-BlackItalic_18pt7b.c \
	display/fonts/font_roboto-BlackItalic_20pt7b.c \
	display/fonts/font_roboto_12pt7b.c \
	display/fonts/font_roboto-BlackItalic_22pt7b.c \
	display/fonts/font_roboto-BlackItalic_24pt7b.c \
	display/fonts/font_roboto-BlackItalic_36pt7b.c \
	display/fonts/font_roboto-BlackItalic_42pt7b.c \
	display/fonts/font_roboto-BlackItalic_6pt7b.c \
	display/fonts/font_roboto-BlackItalic_8pt7b.c \
	display/fonts/font_roboto-BlackItalic_9pt7b.c \
	display/fonts/font_roboto-Black_12pt7b.c \
	display/fonts/font_roboto-Black_13pt7b.c \
	display/fonts/font_roboto-Black_16pt7b.c \
	display/fonts/font_roboto-Black_18pt7b.c \
	display/fonts/font_roboto-Black_20pt7b.c \
	display/fonts/font_roboto-Black_22pt7b.c \
	display/fonts/font_roboto-Black_24pt7b.c \
	display/fonts/font_roboto-Black_36pt7b.c \
	display/fonts/font_roboto-Black_42pt7b.c \
	display/fonts/font_roboto-Black_6pt7b.c \
	display/fonts/font_roboto-Black_8pt7b.c \
	display/fonts/font_roboto-Black_9pt7b.c \
	display/fonts/font_roboto_13pt7b.c \
	display/fonts/font_roboto_16pt7b.c \
	display/fonts/font_roboto_18pt7b.c \
	display/fonts/font_roboto_20pt7b.c \
	display/fonts/font_roboto_22pt7b.c \
	display/fonts/font_roboto_24pt7b.c \
	display/fonts/font_roboto_36pt7b.c \
	display/fonts/font_roboto_42pt7b.c \
	display/fonts/font_roboto_6pt7b.c \
	display/fonts/font_roboto_8pt7b.c \
	display/fonts/font_roboto_9pt7b.c \
	display/fonts/font_weather42pt8b.c \

SRC_CXX += \
	display/driver_framebuffer.cpp \
	display/driver_framebuffer_dirty.cpp \
	display/driver_framebuffer_drawing.cpp \
	display/driver_framebuffer_matrix_drawing.cpp \
	display/driver_framebuffer_matrix.cpp \
	display/driver_framebuffer_orientation.cpp \
	display/driver_framebuffer_text.cpp \
	display/driver_framebuffer_window.cpp \
	display/driver_framebuffer.cpp \

SRC_MOD += moddisplay.c

# source files
SRC_C += \
	main.c \
	gccollect.c \
	unix_mphal.c \
	mpthreadport.c \
	input.c \
	modmachine.c \
	modos.c \
	moduos_vfs.c \
	modtime.c \
	moduselect.c \
	alloc.c \
	fatfs_port.c \
	mpbthciport.c \
	mpbtstackport_common.c \
	mpbtstackport_h4.c \
	mpbtstackport_usb.c \
	mpnimbleport.c \
	$(SRC_MOD) \
	$(wildcard $(VARIANT_DIR)/*.c)

LIB_SRC_C += $(addprefix lib/,\
	$(LIB_SRC_C_EXTRA) \
	timeutils/timeutils.c \
	utils/gchelper_generic.c \
	)

SRC_CXX += \
	$(SRC_MOD_CXX)

OBJ = $(PY_O)
OBJ += $(addprefix $(BUILD)/, $(SRC_C:.c=.o))
OBJ += $(addprefix $(BUILD)/, $(SRC_CXX:.cpp=.o))
OBJ += $(addprefix $(BUILD)/, $(LIB_SRC_C:.c=.o))
OBJ += $(addprefix $(BUILD)/, $(EXTMOD_SRC_C:.c=.o))

# List of sources for qstr extraction
SRC_QSTR += $(SRC_C) $(SRC_CXX) $(LIB_SRC_C) $(EXTMOD_SRC_C)
# Append any auto-generated sources that are needed by sources listed in
# SRC_QSTR
SRC_QSTR_AUTO_DEPS +=

ifneq ($(FROZEN_MANIFEST)$(FROZEN_MPY_DIR),)
# To use frozen code create a manifest.py file with a description of files to
# freeze, then invoke make with FROZEN_MANIFEST=manifest.py (be sure to build from scratch).
CFLAGS += -DMICROPY_QSTR_EXTRA_POOL=mp_qstr_frozen_const_pool
CFLAGS += -DMICROPY_MODULE_FROZEN_MPY
CFLAGS += -DMPZ_DIG_SIZE=16 # force 16 bits to work on both 32 and 64 bit archs
MPY_CROSS_FLAGS += -mcache-lookup-bc
endif

ifneq ($(FROZEN_MANIFEST)$(FROZEN_DIR),)
CFLAGS += -DMICROPY_MODULE_FROZEN_STR
endif

HASCPP17 = $(shell expr `$(CC) -dumpversion | cut -f1 -d.` \>= 7)
ifeq ($(HASCPP17), 1)
	CXXFLAGS += -std=c++17
else
	CXXFLAGS += -std=c++11
endif
CXXFLAGS += $(filter-out -Wmissing-prototypes -Wold-style-definition -std=gnu99,$(CFLAGS) $(CXXFLAGS_MOD))

ifeq ($(MICROPY_FORCE_32BIT),1)
RUN_TESTS_MPY_CROSS_FLAGS = --mpy-cross-flags='-mcache-lookup-bc -march=x86'
else
RUN_TESTS_MPY_CROSS_FLAGS = --mpy-cross-flags='-mcache-lookup-bc'
endif

include $(TOP)/py/mkrules.mk

.PHONY: test test_full

test: $(PROG) $(TOP)/tests/run-tests
	$(eval DIRNAME=ports/$(notdir $(CURDIR)))
	cd $(TOP)/tests && MICROPY_MICROPYTHON=../$(DIRNAME)/$(PROG) ./run-tests

test_full: $(PROG) $(TOP)/tests/run-tests
	$(eval DIRNAME=ports/$(notdir $(CURDIR)))
	cd $(TOP)/tests && MICROPY_MICROPYTHON=../$(DIRNAME)/$(PROG) ./run-tests
	cd $(TOP)/tests && MICROPY_MICROPYTHON=../$(DIRNAME)/$(PROG) ./run-tests -d thread
	cd $(TOP)/tests && MICROPY_MICROPYTHON=../$(DIRNAME)/$(PROG) ./run-tests --emit native
	cd $(TOP)/tests && MICROPY_MICROPYTHON=../$(DIRNAME)/$(PROG) ./run-tests --via-mpy $(RUN_TESTS_MPY_CROSS_FLAGS) -d basics float micropython
	cd $(TOP)/tests && MICROPY_MICROPYTHON=../$(DIRNAME)/$(PROG) ./run-tests --via-mpy $(RUN_TESTS_MPY_CROSS_FLAGS) --emit native -d basics float micropython
	cat $(TOP)/tests/basics/0prelim.py | ./$(PROG) | grep -q 'abc'

test_gcov: test_full
	gcov -o $(BUILD)/py $(TOP)/py/*.c
	gcov -o $(BUILD)/extmod $(TOP)/extmod/*.c

# Value of configure's --host= option (required for cross-compilation).
# Deduce it from CROSS_COMPILE by default, but can be overridden.
ifneq ($(CROSS_COMPILE),)
CROSS_COMPILE_HOST = --host=$(patsubst %-,%,$(CROSS_COMPILE))
else
CROSS_COMPILE_HOST =
endif

deplibs: libffi axtls

libffi: $(BUILD)/lib/libffi/include/ffi.h

$(TOP)/lib/libffi/configure: $(TOP)/lib/libffi/autogen.sh
	cd $(TOP)/lib/libffi; ./autogen.sh

# install-exec-recursive & install-data-am targets are used to avoid building
# docs and depending on makeinfo
$(BUILD)/lib/libffi/include/ffi.h: $(TOP)/lib/libffi/configure
	mkdir -p $(BUILD)/lib/libffi; cd $(BUILD)/lib/libffi; \
	$(abspath $(TOP))/lib/libffi/configure $(CROSS_COMPILE_HOST) --prefix=$$PWD/out --disable-structs CC="$(CC)" CXX="$(CXX)" LD="$(LD)" CFLAGS="-Os -fomit-frame-pointer -fstrict-aliasing -ffast-math -fno-exceptions"; \
	$(MAKE) install-exec-recursive; $(MAKE) -C include install-data-am

axtls: $(TOP)/lib/axtls/README

$(TOP)/lib/axtls/README:
	@echo "You cloned without --recursive, fetching submodules for you."
	(cd $(TOP); git submodule update --init --recursive)

PREFIX = /usr/local
BINDIR = $(DESTDIR)$(PREFIX)/bin

install: $(PROG)
	install -d $(BINDIR)
	install $(PROG) $(BINDIR)/$(PROG)

uninstall:
	-rm $(BINDIR)/$(PROG)
