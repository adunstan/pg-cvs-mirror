# $PostgreSQL: pgsql-server/contrib/userlock/Makefile,v 1.17 2003/11/29 19:51:36 pgsql Exp $

MODULES = user_locks
DATA_built = user_locks.sql
DOCS = README.user_locks

ifdef USE_PGXS
PGXS = $(shell pg_config --pgxs)
include $(PGXS)
else
subdir = contrib/userlock
top_builddir = ../..
include $(top_builddir)/src/Makefile.global
include $(top_srcdir)/contrib/contrib-global.mk
endif
