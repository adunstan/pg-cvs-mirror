# $PostgreSQL: pgsql-server/contrib/tips/Makefile,v 1.6 2003/11/29 19:51:36 pgsql Exp $

DOCS = README.apachelog

ifdef USE_PGXS
PGXS = $(shell pg_config --pgxs)
include $(PGXS)
else
subdir = contrib/tips
top_builddir = ../..
include $(top_builddir)/src/Makefile.global
include $(top_srcdir)/contrib/contrib-global.mk
endif
