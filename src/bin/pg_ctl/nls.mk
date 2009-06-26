# $PostgreSQL: pgsql/src/bin/pg_ctl/nls.mk,v 1.17 2009/04/09 19:38:51 petere Exp $
CATALOG_NAME	:= pg_ctl
AVAIL_LANGUAGES	:= de es fr ja ko pt_BR ru sv ta tr
GETTEXT_FILES	:= pg_ctl.c ../../port/exec.c
GETTEXT_TRIGGERS:= _ simple_prompt
