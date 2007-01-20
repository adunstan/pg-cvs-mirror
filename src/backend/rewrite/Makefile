#-------------------------------------------------------------------------
#
# Makefile--
#    Makefile for rewrite
#
# IDENTIFICATION
#    $PostgreSQL: pgsql/src/backend/rewrite/Makefile,v 1.15 2003/11/29 19:51:55 pgsql Exp $
#
#-------------------------------------------------------------------------

subdir = src/backend/rewrite
top_builddir = ../../..
include $(top_builddir)/src/Makefile.global

OBJS = rewriteRemove.o rewriteDefine.o \
       rewriteHandler.o rewriteManip.o rewriteSupport.o

all: SUBSYS.o

SUBSYS.o: $(OBJS)
	$(LD) $(LDREL) $(LDOUT) SUBSYS.o $(OBJS)

clean: 
	rm -f SUBSYS.o $(OBJS) 
