### Machine flags
#
CC_CMACH	= -mmcu=cc430f6137
CC_DMACH	= -D__MSP430_6137__ -DMRFI_CC430 -D__CC430F6137__
### Build flags
#
CFLAGS		+= $(CC_CMACH) $(CC_DMACH) -Os -Wall -fomit-frame-pointer
CFLAGS		+= -fno-force-addr -finline-limit=1 -fno-schedule-insns
CFLAGS		+= -fshort-enums -ffunction-sections -Wl,-Map=output.map
LDFLAGS		=
INCLUDES	+= -I./ -Igcc/
### Build tools
# 
CC		= msp430-gcc
LD		= msp430-ld
AS		= msp430-as
AR		= msp430-ar
