/* $PostgreSQL: pgsql/contrib/uuid-ossp/uuid-ossp.sql.in,v 1.4 2007/11/13 00:20:36 tgl Exp $ */

-- Adjust this setting to control where the objects get dropped.
SET search_path = public;

DROP FUNCTION ssl_client_serial();
DROP FUNCTION ssl_is_used();
DROP FUNCTION ssl_client_cert_present();
DROP FUNCTION ssl_client_dn_field(text);
DROP FUNCTION ssl_issuer_field(text);
DROP FUNCTION ssl_client_dn();
DROP FUNCTION ssl_issuer_dn();
