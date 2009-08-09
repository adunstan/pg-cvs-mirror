#----------------------------------------------------------------------------
#
# PostgreSQL documentation top-level makefile
#
# Copyright (c) 1994, Regents of the University of California
#
# $PostgreSQL: pgsql/doc/Makefile,v 1.38 2009/08/05 19:31:49 alvherre Exp $
#
#----------------------------------------------------------------------------

subdir = doc
top_builddir = ..
include $(top_builddir)/src/Makefile.global

all distprep html man install installdirs uninstall clean distclean maintainer-clean:
	$(MAKE) -C src $@
