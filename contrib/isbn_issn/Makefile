# $PostgreSQL: pgsql-server/contrib/isbn_issn/Makefile,v 1.12 2003/11/29 19:51:35 pgsql Exp $

MODULES = isbn_issn
DATA_built = isbn_issn.sql
DOCS = README.isbn_issn

ifdef USE_PGXS
PGXS = $(shell pg_config --pgxs)
include $(PGXS)
else
subdir = contrib/isbn_issn
top_builddir = ../..
include $(top_builddir)/src/Makefile.global
include $(top_srcdir)/contrib/contrib-global.mk
endif
