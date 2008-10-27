# $PostgreSQL: pgsql/src/backend/nls.mk,v 1.22 2008/03/24 18:08:47 tgl Exp $
CATALOG_NAME	:= postgres
AVAIL_LANGUAGES	:= af cs de es fr hr hu it ko nb nl pt_BR ro ru sk sl sv tr zh_CN zh_TW
GETTEXT_FILES	:= + gettext-files
GETTEXT_TRIGGERS:= _ errmsg errdetail errdetail_log errhint errcontext write_stderr yyerror

gettext-files: distprep
	find $(srcdir)/ $(srcdir)/../port/ -name '*.c' -print >$@

my-maintainer-clean:
	rm -f gettext-files

.PHONY: my-maintainer-clean
maintainer-clean: my-maintainer-clean
