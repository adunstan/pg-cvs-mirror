#-------------------------------------------------------------------------
#
# Makefile for tools/thread
#
# Copyright (C) 2003 by PostgreSQL Global Development Team
#
# $PostgreSQL: pgsql-server/src/tools/thread/Makefile,v 1.4 2004/04/23 18:15:55 momjian Exp $
#
#-------------------------------------------------------------------------

subdir = tools/thread
top_builddir = ../../..
include $(top_builddir)/src/Makefile.global

override CFLAGS += $(PTHREAD_CFLAGS)

LDFLAGS += $(PTHREAD_LIBS)

all: thread_test

thread_test: thread_test.o
	$(CC) $(CFLAGS) $(LDFLAGS) $^ $(LIBS) -o $@

clean distclean maintainer-clean:
	rm -f thread_test$(X) thread_test.o
