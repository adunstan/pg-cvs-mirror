/* $PostgreSQL: pgsql/contrib/uuid-ossp/uuid-ossp.sql.in,v 1.4 2007/11/13 00:20:36 tgl Exp $ */

-- Adjust this setting to control where the objects get dropped.
SET search_path = public;

DROP FUNCTION uuid_nil();
DROP FUNCTION uuid_ns_dns();
DROP FUNCTION uuid_ns_url();
DROP FUNCTION uuid_ns_oid();
DROP FUNCTION uuid_ns_x500();

DROP FUNCTION uuid_generate_v1();
DROP FUNCTION uuid_generate_v1mc();
DROP FUNCTION uuid_generate_v3(namespace uuid, name text);
DROP FUNCTION uuid_generate_v4();
DROP FUNCTION uuid_generate_v5(namespace uuid, name text);
