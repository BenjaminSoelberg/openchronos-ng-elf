TOP=.
SUBDIRS = drivers modules logic \
simpliciti/Applications/application/End_Device \
simpliciti/Components/nwk \
simpliciti/Components/bsp \
simpliciti/Components/nwk_applications \
simpliciti/Components/mrfi

include $(TOP)/Common.mk

PYTHON := $(shell which python2 || which python)

.PHONY: all
.PHONY: clean
.PHONY: subdirs $(SUBDIRS)
.PHONY: install
.PHONY: config

all: include/config.h eZChronos.txt

eZChronos.elf: even_in_range.o modinit.o $(SUBDIRS)
	@echo -e "\n>> Building $@"
	@$(CC) $(CFLAGS) $(LDFLAGS) $(INCLUDES) -o eZChronos.elf \
		ezchronos.c even_in_range.o modinit.o \
		modules/xbuilt.a logic/xbuilt.a drivers/xbuilt.a \
		simpliciti/Applications/application/End_Device/xbuilt.a \
		simpliciti/Components/nwk/xbuilt.a \
		simpliciti/Components/bsp/xbuilt.a \
		simpliciti/Components/nwk_applications/xbuilt.a \
		simpliciti/Components/mrfi/xbuilt.a

eZChronos.txt: eZChronos.elf
	$(PYTHON) tools/memory.py -i eZChronos.elf -o eZChronos.txt

even_in_range.o: even_in_range.s
	$(AS) $< -o $@

modinit.o: modinit.c
	@echo "CC $<"
	@$(CC) $(CFLAGS) -Wno-implicit-function-declaration \
		$(INCLUDES) -c $< -o $@

modinit.c:
	@echo "Please do a 'make config' first!" && false

include/config.h:
	@echo "Please do a 'make config' first!" && false

$(SUBDIRS):
	@echo -e "\n>> Building $@"
	@$(MAKE) --no-print-directory -C $@ $(MAKECMDGOALS)	

config:
	$(PYTHON) tools/config.py
	$(PYTHON) tools/make_modinit.py
	@echo "Don't forget to do a make clean!" && true

install: eZChronos.txt
	contrib/ChronosTool.py rfbsl eZChronos.txt

clean: $(SUBDIRS)
	rm -f *.o ezChronos.elf ezChronos.txt
