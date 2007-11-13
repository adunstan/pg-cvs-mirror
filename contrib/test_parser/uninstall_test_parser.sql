/* $PostgreSQL: pgsql/contrib/uuid-ossp/uuid-ossp.sql.in,v 1.4 2007/11/13 00:20:36 tgl Exp $ */

-- Adjust this setting to control where the objects get dropped.
SET search_path = public;

DROP TEXT SEARCH PARSER testparser;

DROP FUNCTION testprs_start(internal, int4);

DROP FUNCTION testprs_getlexeme(internal, internal, internal);

DROP FUNCTION testprs_end(internal);

DROP FUNCTION testprs_lextype(internal);
