/*
 * psql - the PostgreSQL interactive terminal
 *
 * Copyright (c) 2000-2004, PostgreSQL Global Development Group
 *
 * $PostgreSQL: pgsql-server/src/bin/psql/psqlscan.h,v 1.1 2004/02/19 19:40:09 tgl Exp $
 */
#ifndef PSQLSCAN_H
#define PSQLSCAN_H

#include "pqexpbuffer.h"

#include "prompt.h"


/* Abstract type for lexer's internal state */
typedef struct PsqlScanStateData *PsqlScanState;

/* Termination states for psql_scan() */
typedef enum
{
	PSCAN_SEMICOLON,			/* found command-ending semicolon */
	PSCAN_BACKSLASH,			/* found backslash command */
	PSCAN_INCOMPLETE,			/* end of line, SQL statement incomplete */
	PSCAN_EOL					/* end of line, SQL possibly complete */
} PsqlScanResult;

/* Different ways for scan_slash_option to handle parameter words */
enum slash_option_type
{
	OT_NORMAL,					/* normal case */
	OT_SQLID,					/* treat as SQL identifier */
	OT_SQLIDHACK,				/* SQL identifier, but don't downcase */
	OT_FILEPIPE,				/* it's a filename or pipe */
	OT_WHOLE_LINE				/* just snarf the rest of the line */
};


extern PsqlScanState psql_scan_create(void);
extern void psql_scan_destroy(PsqlScanState state);

extern void psql_scan_setup(PsqlScanState state,
							const char *line, int line_len);
extern void psql_scan_finish(PsqlScanState state);

extern PsqlScanResult psql_scan(PsqlScanState state,
								PQExpBuffer query_buf,
								promptStatus_t *prompt);

extern void psql_scan_reset(PsqlScanState state);

extern bool psql_scan_in_quote(PsqlScanState state);

extern char *psql_scan_slash_command(PsqlScanState state);

extern char *psql_scan_slash_option(PsqlScanState state,
									enum slash_option_type type,
									char *quote,
									bool semicolon);

extern void psql_scan_slash_command_end(PsqlScanState state);

extern void psql_scan_slash_pushback(PsqlScanState state, const char *str);

#endif   /* PSQLSCAN_H */
