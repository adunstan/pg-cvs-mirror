# $PostgreSQL: pgsql/src/pl/plpython/nls.mk,v 1.1 2008/10/09 17:24:05 alvherre Exp $
CATALOG_NAME	:= plpython
AVAIL_LANGUAGES	:=
GETTEXT_FILES	:= plpython.c
GETTEXT_TRIGGERS:= errmsg errdetail errdetail_log errhint errcontext PLy_elog:2 PLy_exception_set:2
