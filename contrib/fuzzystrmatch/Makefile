# $PostgreSQL: pgsql-server/contrib/fuzzystrmatch/Makefile,v 1.4 2004/07/01 03:25:48 joe Exp $

MODULE_big = fuzzystrmatch
SRCS += fuzzystrmatch.c dmetaphone.c
OBJS = $(SRCS:.c=.o)
DATA_built = fuzzystrmatch.sql
DOCS = README.fuzzystrmatch README.soundex

ifdef USE_PGXS
PGXS = $(shell pg_config --pgxs)
include $(PGXS)
else
subdir = contrib/fuzzystrmatch
top_builddir = ../..
include $(top_builddir)/src/Makefile.global
include $(top_srcdir)/contrib/contrib-global.mk
endif
