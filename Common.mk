### Machine flags
#
CC_CMACH	= -mmcu=cc430f6137
CC_DMACH	= -D__MSP430_6137__ -DMRFI_CC430 -D__CC430F6137__
### Build flags
#
CFLAGS		+= $(CC_CMACH) $(CC_DMACH) -Os -Wall -fomit-frame-pointer
CFLAGS		+= -fno-force-addr -finline-limit=1 -fno-schedule-insns
CFLAGS		+= -fshort-enums -ffunction-sections -Wl,-Map=output.map
CFLAGS		+= -DELIMINATE_BLUEROBIN
CFLAGS		+= $(shell cat include/config.h | grep CONFIG_FREQUENCY | sed 's/.define CONFIG_FREQUENCY //' | sed -e 's/902/-DISM_US/' -e 's/433/-DISM_LF/' -e 's/868/-DISM_EU/')
LDFLAGS		=
INCLUDES	+= -I./
INCLUDES	+= -Iinclude/ -Igcc/
INCLUDES	+= -Idrivers/ -Ilogic/
# this is wrong and shall be removed when modularizarion is complete
INCLUDES	+= -Isimpliciti/
INCLUDES	+= -Isimpliciti/Components/mrfi
INCLUDES	+= -Isimpliciti/Components/bsp
INCLUDES	+= -Isimpliciti/Components/bsp/boards/CC430EM
INCLUDES	+= -Isimpliciti/Components/nwk
INCLUDES	+= -Isimpliciti/Components/nwk_applications
INCLUDES	+= -Isimpliciti/Components/bsp/drivers
### Build tools
# 
CC		= msp430-gcc
LD		= msp430-ld
AS		= msp430-as
AR		= msp430-ar
