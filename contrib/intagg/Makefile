#############################################
# Makefile for integer aggregator
# Copyright (C) 2001 Digital Music Network.
# by Mark L. Woodward
# $PostgreSQL: pgsql-server/contrib/intagg/Makefile,v 1.4 2003/11/29 19:51:35 pgsql Exp $

MODULES = int_aggregate
DATA_built = int_aggregate.sql
DOCS = README.int_aggregate

ifdef USE_PGXS
PGXS = $(shell pg_config --pgxs)
include $(PGXS)
else
subdir = contrib/intagg
top_builddir = ../..
include $(top_builddir)/src/Makefile.global
include $(top_srcdir)/contrib/contrib-global.mk
endif
