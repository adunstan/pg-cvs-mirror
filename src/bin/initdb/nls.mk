# $PostgreSQL: pgsql/src/bin/initdb/nls.mk,v 1.22 2009/10/20 18:23:20 petere Exp $
CATALOG_NAME	:= initdb
AVAIL_LANGUAGES	:= cs de es fr it ja pt_BR ru sv ta tr zh_CN
GETTEXT_FILES	:= initdb.c ../../port/dirmod.c ../../port/exec.c
GETTEXT_TRIGGERS:= _ simple_prompt
