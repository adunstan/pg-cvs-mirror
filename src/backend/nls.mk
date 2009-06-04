# $PostgreSQL: pgsql/src/backend/nls.mk,v 1.25 2009/05/14 21:41:50 alvherre Exp $
CATALOG_NAME	:= postgres
AVAIL_LANGUAGES	:= af cs de es fr hr hu it ja ko nb nl pt_BR ro ru sk sl sv tr zh_CN zh_TW pl
GETTEXT_FILES	:= + gettext-files
GETTEXT_TRIGGERS:= _ errmsg errmsg_plural:1,2 errdetail errdetail_log errdetail_plural:1,2 errhint errcontext write_stderr yyerror

gettext-files: distprep
	find $(srcdir)/ $(srcdir)/../port/ -name '*.c' -print >$@

my-maintainer-clean:
	rm -f gettext-files

.PHONY: my-maintainer-clean
maintainer-clean: my-maintainer-clean
