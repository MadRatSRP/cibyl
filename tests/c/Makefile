######################################################################
##
## Copyright (C) 2005,  Blekinge Institute of Technology
##
## Filename:      Makefile
## Author:        Simon Kagstrom <ska@bth.se>
## Description:   Generic Makefile
##
## $Id: Makefile 15765 2007-07-15 13:48:01Z ska $
##
######################################################################
SUBDIRS =
TARGET  = program
DEFINES = -DVERBOSE=0
SOURCES = src/CRunTime.java src/Main.java src/Syscalls.java
INCLUDES=-I. -I../include -fno-inline-functions
OBJS    = main.o run-tests.o console.o \
          tests/int.o tests/char.o tests/float.o tests/double.o tests/j2me.o \
          tests/varargs.o tests/libm.o tests/memory.o tests/java-exceptions.o \
	  tests/unsigned-int.o tests/file-operations.o tests/jr.o tests/jr_test.o \
          tests/relocation.o tests/astar/main.o tests/astar/finder.o \
          tests/astar/astar.o tests/astar/prio_queue.o tests/slt.o tests/lui.o \
          tests/shifts.o tests/and.o tests/vmips_tests.o tests/vmips/shifts.o \
          tests/vmips/bltz.o tests/vmips/load.o tests/vmips/multu.o tests/vmips/xor.o \
          tests/assign-to-ra.o tests/64-bit-return.o tests/malloc.o tests/function.o \
          tests/tail_call.o tests/return_in_v1.o tests/sw_in_delay_slot.o \
          tests/mul_tests.o tests/long-long.o tests/store-s-regs.o \
          tests/store-s-regs-c-helper.o tests/pass-structs-by-value.o
LDLIBS  = -lm -lsoftfloat -lmidp -ljsr075
EXTRA_CLEAN=*.host.o tests/*.host.o

include $(CIBYL_BASE)/build/Rules.mk

program.host: $(patsubst %.o,%.host.o,$(OBJS))
	gcc -o $@ $+ -lm

%.host.o: %.c
	gcc -DVERBOSE=0 -I. -std=c99 -DHOST=1 -c -Os -Wall -o $@ $<

%.host.o: %.S
	gcc -DVERBOSE=0 -I. -std=c99 -DHOST=1 -c -Os -Wall -o $@ $<
