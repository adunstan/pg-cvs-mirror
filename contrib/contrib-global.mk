# $PostgreSQL: pgsql/contrib/contrib-global.mk,v 1.8 2004/07/30 12:26:39 petere Exp $

NO_PGXS = 1
REGRESS_OPTS = --dbname=$(CONTRIB_TESTDB)
include $(top_srcdir)/src/makefiles/pgxs.mk
