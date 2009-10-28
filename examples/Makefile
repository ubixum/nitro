include ../Makefile 


.PHONY: examples

PROGS=hello serial
CFLAGS=-I../include -g
CPPFLAGS=$(CFLAGS)
LDFLAGS=-L../$(LLIBDIR) -lnitro

examples: $(PROGS)

#$(PROGS): $(OBJS)
