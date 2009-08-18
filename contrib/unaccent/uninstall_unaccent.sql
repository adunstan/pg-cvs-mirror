/* $PostgreSQL: pgsql/contrib/dict_int/uninstall_dict_int.sql,v 1.3 2007/11/13 04:24:27 momjian Exp $ */

DROP FUNCTION IF EXISTS unaccent(regdictionary, text) CASCADE;
DROP FUNCTION IF EXISTS unaccent(text) CASCADE;
DROP TEXT SEARCH DICTIONARY IF EXISTS unaccent CASCADE;
DROP TEXT SEARCH TEMPLATE IF EXISTS unaccent CASCADE;
DROP FUNCTION IF EXISTS unaccent_init(internal) CASCADE;
DROP FUNCTION IF EXISTS unaccent_lexize(internal,internal,internal,internal) CASCADE;

