# $PostgreSQL: pgsql-server/contrib/string/Makefile,v 1.17 2003/11/29 19:51:36 pgsql Exp $

MODULES = string_io
DATA_built = string_io.sql
DOCS = README.string_io

ifdef USE_PGXS
PGXS = $(shell pg_config --pgxs)
include $(PGXS)
else
subdir = contrib/string
top_builddir = ../..
include $(top_builddir)/src/Makefile.global
include $(top_srcdir)/contrib/contrib-global.mk
endif
