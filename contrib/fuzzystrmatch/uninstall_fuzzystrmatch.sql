/* $PostgreSQL: pgsql/contrib/uuid-ossp/uuid-ossp.sql.in,v 1.4 2007/11/13 00:20:36 tgl Exp $ */

-- Adjust this setting to control where the objects get dropped.
SET search_path = public;

DROP FUNCTION dmetaphone_alt (text);

DROP FUNCTION dmetaphone (text);

DROP FUNCTION difference(text,text);

DROP FUNCTION text_soundex(text);

DROP FUNCTION soundex(text);

DROP FUNCTION metaphone (text,int);

DROP FUNCTION levenshtein (text,text);
