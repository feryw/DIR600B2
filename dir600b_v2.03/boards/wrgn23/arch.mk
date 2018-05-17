# vi: set sw=4 ts=4:

#.EXPORT_ALL_VARIABLES:

Q?=@
ifeq ($(strip $(V)),y)
Q:=
endif

ifdef TPATH_UC
CC_PATH := $(TPATH_UC)
else
CC_PATH :=
endif

CROSS_COMPILE	:= mipsel-linux-
HOST_TYPE		:= mipsel-linux
HOST_CPU		:= mips
TARGET_ABI      := 32

CC		= $(CROSS_COMPILE)gcc
CXX		= $(CROSS_COMPILE)g++
AS		= $(CROSS_COMPILE)as
AR		= $(CROSS_COMPILE)ar
LD		= $(CROSS_COMPILE)ld
RANLIB	= $(CROSS_COMPILE)ranlib
STRIP	= $(CROSS_COMPILE)strip
OBJCOPY	= $(CROSS_COMPILE)objcopy

# DO NOT add -DLOGNUM=1 in arch.mk anymore.
# The following ELBOX_TEMPLATE_GW_WIFI case is for backward compatible.
# This option should be add in elbox_config.h & .config by menuconfig.
CFLAGS += -Os -Wall -D__UCLIBC_HAS_MMU__ -D__UCLIBC__
ifeq ($(strip $(ELBOX_TEMPLATE_GW_WIFI)),y)
CFLAGS += -DLOGNUM=1
endif
LDFLAGS:=
CPU_BIG_ENDIAN:=

ifdef TPATH_KC
KCC_PATH:=$(TPATH_KC)
else
KCC_PATH:=
endif
KCC = mipsel-linux-gcc
KLD = mipsel-linux-ld
KCFLAGS = -D__KERNEL__ -Wall -Wstrict-prototypes -Wno-trigraphs -O2 -fno-strict-aliasing -fno-common -fomit-frame-pointer -G 0 -mno-abicalls -fno-pic -pipe  -finline-limit=100000 -march=mips2 -mabi=32 -Wa,--trap -DMODULE -mlong-calls -DWIRELESS_THROUGHPUT_TEST=1 -nostdinc -iwithprefix include
