# Dont forget to set TOP prior including this file!

include $(TOP)/Common.mk

.PHONY: all
.PHONY: clean

SRCS = $(wildcard *.c)
OBJS = $(patsubst %.c, %.o, $(SRCS))

all: xbuilt.a

xbuilt.a: $(OBJS)
	@echo "AR $@"
	@$(AR) rcuos $@ $(OBJS)

%.o: %.c
	@echo "CC $<"
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f *.o xbuilt.a
