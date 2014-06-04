#
# Makefile for cs720/820 assignment
#	Fall 2013
#	Author: Junhong Chen (jmj256)
#	Assignment:


CC = gcc
UNAME_S := $(shell uname -s)

ifeq  ($(OSTYPE),solaris)
  CFLAGS = -Wall -O -g
else
  CFLAGS = -Wall -O -Wextra -g
endif

LDFLAGS = -lpthread


ifeq  ($(OSTYPE),solaris)
  LDFLAGS += -lnsl -lsocket
endif

ifneq  ($(shell uname -s), Darwin)
  LDFLAGS += -lrt
endif

EXECUTABLES = rplcsd rplcs

all:	tags $(EXECUTABLES)

rplcsd:	rplcsd.o search.o functions.o dosearch.o dirf.o dodir.o print_time.o send_recv.o no_sigpipe.o tcpblockio.o remoteserver.o
	$(CC) $^ -o $@ $(LDFLAGS)

*.o: commonincludes.h

rplcs:	rplcs.o search.o functions.o dosearch.o dirf.o dodir.o print_time.o send_recv.o no_sigpipe.o tcpblockio.o remoteclient.o
	$(CC) $^ -o $@ $(LDFLAGS)


*.o: commonincludes.h

tags:	rplcsd.c rplcs.c
	ctags	rplcsd.c rplcs.c

.PHONY:	all clean cleanall

clean:
	$(RM) *.o $(EXECUTABLES) *.log *.tar

cleanall:
	$(RM) *.o $(EXECUTABLES) tags *.log

tar:
	tar cf clientserver.tar *.c *.h Makefile

