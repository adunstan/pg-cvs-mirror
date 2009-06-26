# $PostgreSQL: pgsql/src/bin/initdb/nls.mk,v 1.20 2009/04/09 19:38:50 petere Exp $
CATALOG_NAME	:= initdb
AVAIL_LANGUAGES	:= cs de es fr ja pt_BR ru sv ta tr
GETTEXT_FILES	:= initdb.c ../../port/dirmod.c ../../port/exec.c
GETTEXT_TRIGGERS:= _ simple_prompt
