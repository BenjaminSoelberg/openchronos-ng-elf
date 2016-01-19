### Machine flags
#
CC_CMACH	= -mmcu=cc430f6137
CC_DMACH	= 
### Build flags
#
# -fdata-sections, -ffunction-sections and -Wl,--gc-sections -Wl,-s
# are used for dead code elimination, see:
# http://gcc.gnu.org/ml/gcc-help/2003-08/msg00128.html
#
CFLAGS		+= $(CC_CMACH) $(CC_DMACH) -Wall
CFLAGS		+= -fno-force-addr -finline-limit=1 -fno-schedule-insns
CFLAGS		+= -mhwmult=none -fshort-enums -Wl,-Map=output.map
LDFLAGS		= -L$(MSP430_TI)/include

CFLAGS_REL	+= -Os -fdata-sections -ffunction-sections -fomit-frame-pointer
LDFLAGS_REL	+= -Wl,--gc-sections -Wl,-s

CFLAGS_DBG	+= -O1 -g3 -gdwarf-2 -ggdb
LDFLAGS_DBG	+= -Wl,--gc-sections

# linker flags and include directories
INCLUDES	+= -I./ -I$(MSP430_TI)/include
### Build tools
# 
CC		= msp430-elf-gcc
LD		= msp430-elf-ld
AS		= msp430-elf-as
AR		= msp430-elf-ar
