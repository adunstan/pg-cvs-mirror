/* $PostgreSQL: pgsql/contrib/uuid-ossp/uuid-ossp.sql.in,v 1.4 2007/11/13 00:20:36 tgl Exp $ */

-- Adjust this setting to control where the objects get dropped.
SET search_path = public;

DROP TEXT SEARCH DICTIONARY intdict;

DROP TEXT SEARCH TEMPLATE intdict_template;

DROP FUNCTION dintdict_init(internal);

DROP FUNCTION dintdict_lexize(internal,internal,internal,internal);
