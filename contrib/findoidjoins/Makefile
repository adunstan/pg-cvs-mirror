# $PostgreSQL: pgsql-server/contrib/findoidjoins/Makefile,v 1.15 2003/11/29 19:51:35 pgsql Exp $

PROGRAM = findoidjoins
OBJS	= findoidjoins.o

PG_CPPFLAGS = -I$(libpq_srcdir)
PG_LIBS = $(libpq)

SCRIPTS = make_oidjoins_check
DOCS = README.findoidjoins

ifdef USE_PGXS
PGXS = $(shell pg_config --pgxs)
include $(PGXS)
else
subdir = contrib/findoidjoins
top_builddir = ../..
include $(top_builddir)/src/Makefile.global
include $(top_srcdir)/contrib/contrib-global.mk
endif
