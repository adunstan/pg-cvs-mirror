/* $PostgreSQL: pgsql/contrib/uuid-ossp/uuid-ossp.sql.in,v 1.4 2007/11/13 00:20:36 tgl Exp $ */

-- Adjust this setting to control where the objects get dropped.
SET search_path = public;

DROP FUNCTION int_array_enum(int4[]);

DROP AGGREGATE int_array_aggregate (int4);

DROP FUNCTION int_agg_final_array (int4[]);

DROP FUNCTION int_agg_state (int4[], int4);
