# $PostgreSQL: pgsql-server/contrib/noupdate/Makefile,v 1.10 2003/11/29 19:51:35 pgsql Exp $

MODULES = noup
DATA_built = noup.sql
DOCS = README.noup

ifdef USE_PGXS
PGXS = $(shell pg_config --pgxs)
include $(PGXS)
else
subdir = contrib/noupdate
top_builddir = ../..
include $(top_builddir)/src/Makefile.global
include $(top_srcdir)/contrib/contrib-global.mk
endif
