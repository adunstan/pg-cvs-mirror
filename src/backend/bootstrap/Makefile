#-------------------------------------------------------------------------
#
# Makefile for the bootstrap module
#
# $PostgreSQL: pgsql/src/backend/bootstrap/Makefile,v 1.38 2009/08/28 20:26:18 petere Exp $
#
#-------------------------------------------------------------------------

subdir = src/backend/bootstrap
top_builddir = ../../..
include $(top_builddir)/src/Makefile.global

override CPPFLAGS := -I. -I$(srcdir) $(CPPFLAGS)

OBJS= bootparse.o bootstrap.o 

include $(top_srcdir)/src/backend/common.mk


# bootscanner is compiled as part of bootparse
bootparse.o: bootscanner.c

bootparse.c: bootparse.y
ifdef BISON
	$(BISON) $(BISONFLAGS) -o $@ $<
else
	@$(missing) bison $< $@
endif

bootscanner.c: bootscanner.l
ifdef FLEX
	$(FLEX) $(FLEXFLAGS) -o'$@' $<
else
	@$(missing) flex $< $@
endif


# bootparse.c and bootscanner.c are in the distribution tarball, so
# they are not cleaned here.
