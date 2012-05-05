SUBDIRS = drivers modules logic \
simpliciti/Applications/application/End_Device \
simpliciti/Components/nwk \
simpliciti/Components/bsp \
simpliciti/Components/nwk_applications \
simpliciti/Components/mrfi

include Common.mk

PYTHON := $(shell which python2 || which python)

.PHONY: all
.PHONY: clean
.PHONY: install
.PHONY: config

all: include/config.h ezchronos.txt

#
# Build list of archives to be built in
BUILTIN := $(patsubst %,%/xbuilt.a,$(SUBDIRS))

#
# Make list of object dependency for each archive
define XBUILT_RULE
$(1): $(2)
	@echo "AR $$@"
	@$(AR) rcuTPs $$@ $$+
endef

$(foreach subdir,$(SUBDIRS), \
	$(eval $(call XBUILT_RULE,\
		$(subdir)/xbuilt.a,\
		$(subst .c,.o,$(wildcard $(subdir)/*.c)) \
	)) \
)

ezchronos.elf: even_in_range.o modinit.o ezchronos.o $(BUILTIN)
	@echo -e "\n>> Building $@"
	@$(CC) $(CFLAGS) $(LDFLAGS) $(INCLUDES) -o $@ $+	

ezchronos.txt: ezchronos.elf
	$(PYTHON) tools/memory.py -i $< -o $@

even_in_range.o: even_in_range.s
	$(AS) $< -o $@

modinit.o: modinit.c
	@echo "CC $<"
	@$(CC) $(CFLAGS) -Wno-implicit-function-declaration \
		$(INCLUDES) -c $< -o $@

%.o: %.c
	@echo "CC $<"
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

modinit.c:
	@echo "Please do a 'make config' first!" && false

include/config.h:
	@echo "Please do a 'make config' first!" && false

config:
	$(PYTHON) tools/config.py
	$(PYTHON) tools/make_modinit.py
	@echo "Don't forget to do a make clean!" && true

install: ezchronos.txt
	contrib/ChronosTool.py rfbsl $<

clean: $(SUBDIRS)
	@for subdir in $(SUBDIRS); do \
		echo "Cleaning $$subdir .."; rm -f $$subdir/*.{o,.a}; \
	done
	@rm -f *.o ezchronos.elf ezchronos.txt
