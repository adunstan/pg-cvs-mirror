#-------------------------------------------------------------------------
#
# Makefile for utils
#
# $PostgreSQL: pgsql-server/src/utils/Makefile,v 1.20 2003/11/29 19:52:15 pgsql Exp $
#
# dllinit.o is only built on Win32 platform.
#
#-------------------------------------------------------------------------

subdir = src/utils
top_builddir = ../..
include $(top_builddir)/src/Makefile.global


ifdef DLLINIT

all:

install: all installdirs
	$(INSTALL_DATA) dllinit.o $(DESTDIR)$(pgxsdir)/$(subdir)/

installdirs:
	$(mkinstalldirs) $(DESTDIR)$(pgxsdir)/$(subdir)

uninstall:
	rm -f $(DESTDIR)$(pgxsdir)/$(subdir)/dllinit.o

clean distclean maintainer-clean:
	rm -f dllinit.o

endif # not DLLINIT
