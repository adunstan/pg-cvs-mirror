# $PostgreSQL: pgsql-server/contrib/miscutil/Makefile,v 1.17 2003/11/29 19:51:35 pgsql Exp $

MODULES = misc_utils
DATA_built = misc_utils.sql
DOCS = README.misc_utils

ifdef USE_PGXS
PGXS = $(shell pg_config --pgxs)
include $(PGXS)
else
subdir = contrib/miscutil
top_builddir = ../..
include $(top_builddir)/src/Makefile.global
include $(top_srcdir)/contrib/contrib-global.mk
endif
