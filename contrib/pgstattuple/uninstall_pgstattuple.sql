/* $PostgreSQL: pgsql/contrib/uuid-ossp/uuid-ossp.sql.in,v 1.4 2007/11/13 00:20:36 tgl Exp $ */

-- Adjust this setting to control where the objects get dropped.
SET search_path = public;

DROP FUNCTION pgstattuple(text);
DROP FUNCTION pgstattuple(oid);
DROP FUNCTION pgstatindex(text);
DROP FUNCTION pg_relpages(text);
