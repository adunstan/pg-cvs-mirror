#
# Makefile for pg_upgrade_support
#
# $PostgreSQL: pgsql/contrib/citext/Makefile,v 1.1 2008/07/29 18:31:20 tgl Exp $

PGFILEDESC = "pg_upgrade_support - server-side functions for pg_upgrade"

MODULES = pg_upgrade_support

ifdef USE_PGXS
PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
else
subdir = contrib/pg_upgrade_support
top_builddir = ../..
include $(top_builddir)/src/Makefile.global
include $(top_srcdir)/contrib/contrib-global.mk
endif
