#-------------------------------------------------------------------------
#
# pgstattuple Makefile
#
# $PostgreSQL: pgsql-server/contrib/pgstattuple/Makefile,v 1.2 2003/11/29 22:39:29 pgsql Exp $
#
#-------------------------------------------------------------------------

SRCS		= pgstattuple.c

MODULE_big	= pgstattuple
OBJS		= $(SRCS:.c=.o)
DOCS		= README.pgstattuple README.pgstattuple.euc_jp
DATA_built	= pgstattuple.sql

ifdef USE_PGXS
PGXS = $(shell pg_config --pgxs)
include $(PGXS)
else
subdir = contrib/pgstattuple
top_builddir = ../..
include $(top_builddir)/src/Makefile.global
include $(top_srcdir)/contrib/contrib-global.mk
endif

