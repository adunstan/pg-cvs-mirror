# $PostgreSQL: pgsql-server/contrib/fuzzystrmatch/Makefile,v 1.3 2003/11/29 19:51:35 pgsql Exp $

subdir = contrib/fuzzystrmatch
top_builddir = ../..
include $(top_builddir)/src/Makefile.global

MODULE_big = fuzzystrmatch
SRCS += fuzzystrmatch.c dmetaphone.c
OBJS = $(SRCS:.c=.o)
DATA_built = fuzzystrmatch.sql
DOCS = README.fuzzystrmatch README.soundex

include $(top_srcdir)/contrib/contrib-global.mk
