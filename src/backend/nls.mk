# $PostgreSQL: pgsql-server/src/backend/nls.mk,v 1.12 2004/06/10 17:10:24 petere Exp $
CATALOG_NAME	:= postgres
AVAIL_LANGUAGES	:= af cs de es hr hu it nb pt_BR ru sv tr zh_CN zh_TW
GETTEXT_FILES	:= + gettext-files
# you can add "elog:2" and "errmsg_internal" to this list if you want to
# include internal messages in the translation list.
GETTEXT_TRIGGERS:= errmsg errdetail errhint errcontext write_stderr yyerror

gettext-files: distprep
	find $(srcdir)/ -name '*.c' -print >$@

my-maintainer-clean:
	rm -f gettext-files

.PHONY: my-maintainer-clean
maintainer-clean: my-maintainer-clean
