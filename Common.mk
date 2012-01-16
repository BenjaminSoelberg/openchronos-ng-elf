### Machine flags
#
CC_CMACH	= -mmcu=cc430f6137
CC_DMACH	= -D__MSP430_6137__ -DMRFI_CC430 -D__CC430F6137__
### Build flags
#
CFLAGS		+= $(CC_CMACH) $(CC_DMACH) -Os -Wall -fomit-frame-pointer
CFLAGS		+= -fno-force-addr -finline-limit=1 -fno-schedule-insns
CFLAGS		+= -fshort-enums -Wl,-Map=output.map
CFLAGS		+= -DELIMINATE_BLUEROBIN
LDFLAGS		=
INCLUDES	+= -I$(TOP)/
INCLUDES	+= -I$(TOP)/include/ -I$(TOP)/gcc/
INCLUDES	+= -I$(TOP)/drivers/ -I$(TOP)/logic/
INCLUDES	+= -I$(TOP)/simpliciti/
INCLUDES	+= -I$(TOP)/simpliciti/Components/mrfi
INCLUDES	+= -I$(TOP)/simpliciti/Components/bsp
INCLUDES	+= -I$(TOP)/simpliciti/Components/bsp/boards/CC430EM
INCLUDES	+= -I$(TOP)/simpliciti/Components/nwk
INCLUDES	+= -I$(TOP)/simpliciti/Components/nwk_applications
INCLUDES	+= -I$(TOP)/simpliciti/Components/bsp/drivers
# this is wrong and shall be removed when modularizarion is complete
INCLUDES	+= -I$(TOP)/modules/
### Build tools
# 
CC 		= msp430-gcc
LD		= msp430-ld
AS		= msp430-as
