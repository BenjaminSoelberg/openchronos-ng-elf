TOP=.
SUBDIRS = drivers modules logic simpliciti

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
		ezchronos.c even_in_range.o drivers/xbuilt.o \
		modules/xbuilt.o logic/xbuilt.o simpliciti/xbuilt.o

eZChronos.txt: eZChronos.elf
	$(PYTHON) tools/memory.py -i eZChronos.elf -o eZChronos.txt

even_in_range.o: even_in_range.s
	$(AS) $< -o $@

modinit.o: modinit.c
	@echo "CC $<"
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

modinit.c:
	@echo "Please do a 'make config' first!" && false

include/config.h:
	@echo "Please do a 'make config' first!" && false

$(SUBDIRS):
	@echo -e "\n>> Building $@"
	@$(MAKE) -C $@ $(MAKECMDGOALS)	

config:
	$(PYTHON) tools/config.py
	$(PYTHON) tools/make_modinit.py
	@echo "Don't forget to do a make clean!" && true

install: eZChronos.txt
	contrib/ChronosTool.py rfbsl eZChronos.txt

clean: $(SUBDIRS)
	rm -f *.o ezChronos.elf ezChronos.txt
