TOP=../../..

CFLAGS = $(shell cat $(TOP)/include/config.h | grep CONFIG_FREQUENCY | sed 's/.define CONFIG_FREQUENCY //' | sed 's/902/-DISM_US/' | sed 's/433/-DISM_LF/' | sed 's/868/-DISM_EU/')

include $(TOP)/Rules.mk
