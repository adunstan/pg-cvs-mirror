# $PostgreSQL: pgsql/config/Makefile,v 1.2 2005/12/09 21:19:34 petere Exp $

subdir = config
top_builddir = ..
include $(top_builddir)/src/Makefile.global


install: all installdirs
	$(INSTALL_SCRIPT) $(srcdir)/install-sh '$(DESTDIR)$(pgxsdir)/config/install-sh'

installdirs:
	$(MKDIR_P) '$(DESTDIR)$(pgxsdir)/config'

uninstall:
	rm -f '$(DESTDIR)$(pgxsdir)/config/install-sh'
