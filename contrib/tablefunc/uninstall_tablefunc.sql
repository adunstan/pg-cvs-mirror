/* $PostgreSQL: pgsql/contrib/uuid-ossp/uuid-ossp.sql.in,v 1.4 2007/11/13 00:20:36 tgl Exp $ */

-- Adjust this setting to control where the objects get dropped.
SET search_path = public;

DROP FUNCTION connectby(text,text,text,text,text,int);

DROP FUNCTION connectby(text,text,text,text,text,int,text);

DROP FUNCTION connectby(text,text,text,text,int);

DROP FUNCTION connectby(text,text,text,text,int,text);

DROP FUNCTION crosstab(text,text);

DROP FUNCTION crosstab(text,int);

DROP FUNCTION crosstab4(text);

DROP FUNCTION crosstab3(text);

DROP FUNCTION crosstab2(text);

DROP TYPE tablefunc_crosstab_4;

DROP TYPE tablefunc_crosstab_3;

DROP TYPE tablefunc_crosstab_2;

DROP FUNCTION crosstab(text);

DROP FUNCTION normal_rand(int4, float8, float8);
