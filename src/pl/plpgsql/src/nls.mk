# $PostgreSQL: pgsql/src/pl/plpgsql/src/nls.mk,v 1.2 2008/10/09 18:15:28 alvherre Exp $
CATALOG_NAME	:= plpgsql
AVAIL_LANGUAGES	:= es
GETTEXT_FILES	:= pl_comp.c pl_exec.c pl_gram.c pl_funcs.c pl_handler.c pl_scan.c
GETTEXT_TRIGGERS:= _ errmsg errdetail errdetail_log errhint errcontext yyerror

.PHONY: gettext-files
gettext-files: distprep
