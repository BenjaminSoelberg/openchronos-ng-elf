TOP=..
SUBDIRS = \
Components/bsp \
Components/mrfi \
Components/nwk \
Components/nwk_applications \
Applications/application/End_Device

include $(TOP)/Common.mk

.PHONY: all
.PHONY: clean
.PHONY: subdirs $(SUBDIRS)

OBJS = Components/bsp/bsp.o \
Components/mrfi/mrfi.o \
Components/nwk/xbuilt.o \
Components/nwk_applications/xbuilt.o \
Applications/application/End_Device/xbuilt.o

all: xbuilt.o

xbuilt.o: $(SUBDIRS)
	@echo "LD $@"
	@$(LD) -r $(OBJS) -o $@

$(SUBDIRS):
	@echo -e "\n>> Building simplicity/$@"
	@$(MAKE) -C $@ $(MAKECMDGOALS)

clean: $(SUBDIRS)
	rm -f *.o
