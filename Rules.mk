# Dont forget to set TOP prior including this file!

include $(TOP)/Common.mk

.PHONY: all
.PHONY: clean

SRCS = $(wildcard *.c)
OBJS = $(patsubst %.c, %.o, $(SRCS))

all: xbuilt.o

xbuilt.o: $(OBJS)
	$(LD) -r $(OBJS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f *.o
