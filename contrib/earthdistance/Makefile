# $PostgreSQL: contrib/earthdistance/Makefile,v 1.13 2003/11/29 19:51:35 pgsql Exp $

MODULES = earthdistance
DATA_built = earthdistance.sql
DOCS = README.earthdistance
REGRESS = earthdistance

ifdef USE_PGXS
PGXS = $(shell pg_config --pgxs)
include $(PGXS)
else
subdir = contrib/earthdistance
top_builddir = ../..
include $(top_builddir)/src/Makefile.global
include $(top_srcdir)/contrib/contrib-global.mk
endif
