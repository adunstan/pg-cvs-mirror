# $PostgreSQL: pgsql/src/pl/plperl/nls.mk,v 1.5 2009/05/14 21:41:53 alvherre Exp $
CATALOG_NAME	:= plperl
AVAIL_LANGUAGES	:= de es fr pt_BR tr
GETTEXT_FILES	:= plperl.c SPI.c
GETTEXT_TRIGGERS:= errmsg errmsg_plural:1,2 errdetail errdetail_log errdetail_plural:1,2 errhint errcontext
