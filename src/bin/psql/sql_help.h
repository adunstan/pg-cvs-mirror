/*
 * This file is automatically generated from the SGML documentation.
 * Direct changes here will be overwritten.
 */
#ifndef SQL_HELP_H
#define SQL_HELP_H

struct _helpStruct
{
	char	   *cmd;			/* the command name */
	char	   *help;			/* the help associated with it */
	char	   *syntax;			/* the syntax associated with it */
};


static struct _helpStruct QL_HELP[] = {
	{"TRUNCATE",
		"Empty a table",
	"TRUNCATE [ TABLE ] name"},

	{"ABORT",
		"Aborts the current transaction",
	"ABORT [ WORK | TRANSACTION ]"},

	{"ALTER TABLE",
		"Modifies table properties",
	"ALTER TABLE table\n    [ * ] ADD [ COLUMN ] ER\">coBLE> type\nALTER TABLE table\n    [ * ] RENAME [ COLUMN ] ER\">coBLE> TO newcolumn\nALTER TABLE table\n    RENAME TO newtable"},

	{"ALTER USER",
		"Modifies user account information",
	"ALTER USER username [ WITH PASSWORD password ]\n    [ CREATEDB | NOCREATEDB ] [ CREATEUSER | NOCREATEUSER ]\n    [ IN GROUP groupname [, ...] ]\n    [ VALID UNTIL 'abstime' ]"},

	{"BEGIN",
		"Begins a transaction in chained mode",
	"BEGIN [ WORK | TRANSACTION ]"},

	{"CLOSE",
		"Close a cursor",
	"CLOSE cursor"},

	{"CLUSTER",
		"Gives storage clustering advice to the server",
	"CLUSTER indexname ON table"},

	{"COMMIT",
		"Commits the current transaction",
	"COMMIT [ WORK | TRANSACTION ]"},

	{"COPY",
		"Copies data between files and tables",
	"COPY [ BINARY ] table [ WITH OIDS ]\n    FROM { 'filename' | stdin }\n    [ [USING] DELIMITERS 'delimiter' ]\nCOPY [ BINARY ] table [ WITH OIDS ]\n    TO { 'filename' | stdout }\n    [ [USING] DELIMITERS 'delimiter' ]"},

	{"CREATE AGGREGATE",
		"Defines a new aggregate function",
	"CREATE AGGREGATE name [ AS ] ( BASETYPE = data_type\n    [ , SFUNC1 = sfunc1, STYPE1 = sfunc1_return_type ]\n    [ , SFUNC2 = sfunc2, STYPE2 = sfunc2_return_type ]\n    [ , FINALFUNC = ffunc ]\n    [ , INITCOND1 = initial_condition1 ]\n    [ , INITCOND2 = initial_condition2 ] )"},

	{"CREATE DATABASE",
		"Creates a new database",
	"CREATE DATABASE name [ WITH LOCATION = 'dbpath' ]"},

	{"CREATE FUNCTION",
		"Defines a new function",
	"CREATE FUNCTION name ( [ ftype [, ...] ] )\n    RETURNS rtype\n    [ WITH ( attribute [, ...] ) ]\n    AS definition   \n    LANGUAGE 'langname'\n\n\nCREATE FUNCTION name ( [ ftype [, ...] ] )\n    RETURNS rtype\n    [ WITH ( attribute [, ...] ) ]\n    AS obj_file , link_symbol  \n    LANGUAGE 'C'"},

	{"CREATE INDEX",
		"Constructs a secondary index",
	"CREATE [ UNIQUE ] INDEX index_name ON table\n    [ USING acc_name ] ( column [ ops_name] [, ...] )\nCREATE [ UNIQUE ] INDEX index_name ON table\n    [ USING acc_name ] ( func_name( r\">colle> [, ... ]) ops_name )"},

	{"CREATE LANGUAGE",
		"Defines a new language for functions",
	"CREATE [ TRUSTED ] PROCEDURAL LANGUAGE 'langname'\n    HANDLER call_handler\n    LANCOMPILER 'comment'"},

	{"CREATE OPERATOR",
		"Defines a new user operator",
	"CREATE OPERATOR name ( PROCEDURE = func_name\n     [, LEFTARG = type1 ] [, RIGHTARG = type2 ]\n     [, COMMUTATOR = com_op ] [, NEGATOR = neg_op ]\n     [, RESTRICT = res_proc ] [, JOIN = join_proc ]\n     [, HASHES ] [, SORT1 = left_sort_op ] [, SORT2 = right_sort_op ] )"},

	{"CREATE RULE",
		"Defines a new rule",
	"CREATE RULE name AS ON event\n    TO object [ WHERE condition ]\n    DO [ INSTEAD ] [ action | NOTHING ]"},

	{"CREATE SEQUENCE",
		"Creates a new sequence number generator",
	"CREATE SEQUENCE seqname [ INCREMENT increment ]\n    [ MINVALUE minvalue ] [ MAXVALUE maxvalue ]\n    [ START start ] [ CACHE cache ] [ CYCLE ]"},

	{"CREATE TABLE",
		"Creates a new table",
	"CREATE [ TEMPORARY | TEMP ] TABLE table (\n    column type\n    [ NULL | NOT NULL ] [ UNIQUE ] [ DEFAULT value ]\n    [column_constraint_clause | PRIMARY KEY } [ ... ] ]\n    [, ... ]\n    [, PRIMARY KEY ( column [, ...] ) ]\n    [, CHECK ( condition ) ]\n    [, table_constraint_clause ]\n    ) [ INHERITS ( inherited_table [, ...] ) ]"},

	{"CREATE TABLE AS",
		"Creates a new table",
	"CREATE TABLE table [ (column [, ...] ) ]\n     AS select_clause"},

	{"CREATE TRIGGER",
		"Creates a new trigger",
	"CREATE TRIGGER name { BEFORE | AFTER } { event [OR ...] }\n    ON table FOR EACH { ROW | STATEMENT }\n    EXECUTE PROCEDURE ER\">funcBLE> ( arguments )"},

	{"CREATE TYPE",
		"Defines a new base data type",
	"CREATE TYPE typename ( INPUT = input_function, OUTPUT = output_function\n      , INTERNALLENGTH = { internallength | VARIABLE } [ , EXTERNALLENGTH = { externallength | VARIABLE } ]\n    [ , DEFAULT = \"default\" ]\n    [ , ELEMENT = element ] [ , DELIMITER = delimiter ]\n    [ , SEND = send_function ] [ , RECEIVE = receive_function ]\n    [ , PASSEDBYVALUE ] )"},

	{"CREATE USER",
		"Creates account information for a new user",
	"CREATE USER username\n    [ WITH PASSWORD password ]\n    [ CREATEDB   | NOCREATEDB ] [ CREATEUSER | NOCREATEUSER ]\n    [ IN GROUP     groupname [, ...] ]\n    [ VALID UNTIL  'abstime' ]"},

	{"CREATE VIEW",
		"Constructs a virtual table",
	"CREATE VIEW view AS SELECT query"},

	{"DECLARE",
		"Defines a cursor for table access",
	"DECLARE cursor [ BINARY ] [ INSENSITIVE ] [ SCROLL ]\n    CURSOR FOR query\n    [ FOR { READ ONLY | UPDATE [ OF column [, ...] ] ]"},

	{"DELETE",
		"Removes rows from a table",
	"DELETE FROM table [ WHERE condition ]"},

	{"DROP AGGREGATE",
		"Removes the definition of an aggregate function",
	"DROP AGGREGATE name type"},

	{"FETCH",
		"Gets rows using a cursor",
	"FETCH [ selector ] [ count ] { IN | FROM } cursor\nFETCH [ RELATIVE ] [ { [ # | ALL | NEXT | PRIOR ] } ] FROM ] cursor"},

	{"DROP DATABASE",
		"Destroys an existing database",
	"DROP DATABASE name"},

	{"DROP FUNCTION",
		"Removes a user-defined C function",
	"DROP FUNCTION name ( [ type [, ...] ] )"},

	{"DROP INDEX",
		"Removes an index from a database",
	"DROP INDEX index_name"},

	{"DROP LANGUAGE",
		"Removes a user-defined procedural language",
	"DROP PROCEDURAL LANGUAGE 'name'"},

	{"DROP OPERATOR",
		"Removes an operator from the database",
	"DROP OPERATOR id ( type | NONE [,...] )"},

	{"DROP RULE",
		"Removes an existing rule from the database",
	"DROP RULE name"},

	{"DROP SEQUENCE",
		"Removes an existing sequence",
	"DROP SEQUENCE name [, ...]"},

	{"DROP TABLE",
		"Removes existing tables from a database",
	"DROP TABLE name [, ...]"},

	{"DROP TRIGGER",
		"Removes the definition of a trigger",
	"DROP TRIGGER name ON table"},

	{"DROP TYPE",
		"Removes a user-defined type from the system catalogs",
	"DROP TYPE typename"},

	{"DROP USER",
		"Removes an user account information",
	"DROP USER name"},

	{"DROP VIEW",
		"Removes an existing view from a database",
	"DROP VIEW name"},

	{"EXPLAIN",
		"Shows statement execution details",
	"EXPLAIN [ VERBOSE ] query"},

	{"GRANT",
		"Grants access privilege to a user, a group or all users",
	"GRANT privilege [, ...] ON object [, ...]\n    TO { PUBLIC | GROUP group | username }"},

	{"INSERT",
		"Inserts new rows into a table",
	"INSERT INTO table [ ( column [, ...] ) ]\n    { VALUES ( expression [, ...] ) | SELECT query }"},

	{"LISTEN",
		"Listen for a response on a notify condition",
	"LISTEN name"},

	{"LOAD",
		"Dynamically loads an object file",
	"LOAD 'filename'"},

	{"LOCK",
		"Explicitly lock a table inside a transaction",
	"LOCK [ TABLE ] name\nLOCK [ TABLE ] name IN [ ROW | ACCESS ] { SHARE | EXCLUSIVE } MODE\nLOCK [ TABLE ] name IN SHARE ROW EXCLUSIVE MODE"},

	{"MOVE",
		"Moves cursor position",
	"MOVE [ selector ] [ count ] \n    { IN | FROM } cursor\n    FETCH [ RELATIVE ] [ { [ # | ALL | NEXT | PRIOR ] } ] FROM ] cursor"},

	{"NOTIFY",
		"Signals all frontends and backends listening on a notify condition",
	"NOTIFY name"},

	{"RESET",
		"Restores run-time parameters for session to default values",
	"RESET variable"},

	{"REVOKE",
		"Revokes access privilege from a user, a group or all users.",
	"REVOKE privilege [, ...]\n    ON object [, ...]\n    FROM { PUBLIC | GROUP ER\">gBLE> | username }"},

	{"ROLLBACK",
		"Aborts the current transaction",
	"ROLLBACK [ WORK | TRANSACTION ]"},

	{"SELECT",
		"Retrieve rows from a table or view.",
	"SELECT [ ALL | DISTINCT [ ON column ] ]\n    expression [ AS name ] [, ...]\n    [ INTO [ TEMPORARY | TEMP ] [ TABLE ] new_table ]\n    [ FROM table [ alias ] [, ...] ]\n    [ WHERE condition ]\n    [ GROUP BY column [, ...] ]\n    [ HAVING condition [, ...] ]\n    [ { UNION [ ALL ] | INTERSECT | EXCEPT } select ]\n    [ ORDER BY column [ ASC | DESC ] [, ...] ]\n    [ FOR UPDATE [ OF class_name... ] ]\n    [ LIMIT { count | ALL } [ { OFFSET | , } count ] ]"},

	{"SELECT INTO",
		"Create a new table from an existing table or view",
	"SELECT [ ALL | DISTINCT ] expression [ AS name ] [, ...]\n    INTO [TEMP] [ TABLE ] new_table ]\n    [ FROM table [alias] [, ...] ]\n    [ WHERE condition ]\n    [ GROUP BY column [, ...] ]\n    [ HAVING condition [, ...] ]\n    [ { UNION [ALL] | INTERSECT | EXCEPT } select]\n    [ ORDER BY column [ ASC | DESC ] [, ...] ]\n    [ FOR UPDATE [OF class_name...]]\n    [ LIMIT count [OFFSET|, count]]"},

	{"SET",
		"Set run-time parameters for session",
	"SET variable { TO | = } { 'value' | DEFAULT }\nSET TIME ZONE { 'timezone' | LOCAL | DEFAULT }\nSET TRANSACTION ISOLATION LEVEL { READ COMMITTED | SERIALIZABLE }"},

	{"SHOW",
		"Shows run-time parameters for session",
	"SHOW keyword"},

	{"UNLISTEN",
		"Stop listening for notification",
	"UNLISTEN { notifyname | * }"},

	{"UPDATE",
		"Replaces values of columns in a table",
	"UPDATE table SET R\">colle> = expression [, ...]\n    [ FROM fromlist ]\n    [ WHERE condition ]"},

	{"VACUUM",
		"Clean and analyze a Postgres database",
	"VACUUM [ VERBOSE ] [ ANALYZE ] [ table ]\nVACUUM [ VERBOSE ] ANALYZE [ ER\">tBLE> [ (column [, ...] ) ] ]"},

	{"END",
		"Commits the current transaction",
	"END [ WORK | TRANSACTION ]"},

	{"COMMENT",
		"Add comment to an object",
	"COMMENT ON\n[\n  [ DATABASE | INDEX | RULE | SEQUENCE | TABLE | TYPE | VIEW ]\n  object_name |\n  COLUMN table_name.column_name|\n  AGGREGATE agg_name agg_type|\n  FUNCTION func_name (arg1, arg2, ...)|\n  OPERATOR op (leftoperand_type rightoperand_type) |\n  TRIGGER trigger_name ON table_name\n] IS 'text'"},


	{NULL, NULL, NULL}			/* End of list marker */
};

#endif	 /* SQL_HELP_H */
