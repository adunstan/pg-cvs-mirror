# $PostgreSQL: pgsql-server/contrib/dbmirror/Makefile,v 1.2 2003/11/29 19:51:34 pgsql Exp $

MODULES = pending
DOCS = README.dbmirror

ifdef USE_PGXS
PGXS = $(shell pg_config --pgxs)
include $(PGXS)
else
subdir = contrib/dbmirror
top_builddir = ../..
include $(top_builddir)/src/Makefile.global
include $(top_srcdir)/contrib/contrib-global.mk
endif
