# $PostgreSQL: pgsql/src/pl/tcl/nls.mk,v 1.4 2009/05/14 21:41:53 alvherre Exp $
CATALOG_NAME	:= pltcl
AVAIL_LANGUAGES	:= de es fr pt_BR tr
GETTEXT_FILES	:= pltcl.c
GETTEXT_TRIGGERS:= errmsg errmsg_plural:1,2 errdetail errdetail_log errdetail_plural:1,2 errhint errcontext
