
/* File:            bind.h
 *
 * Description:     See "bind.c"
 *
 * Comments:        See "notice.txt" for copyright and license information.
 *
 */

#ifndef __BIND_H__
#define __BIND_H__

#include "psqlodbc.h"

/*
 * BindInfoClass -- stores information about a bound column
 */
struct BindInfoClass_ {
	Int4 buflen;		/* size of buffer */
	char *buffer;		/* pointer to the buffer */
	Int4 *used;			/* used space in the buffer (for strings not counting the '\0') */
	Int2 returntype;	/* kind of conversion to be applied when returning (SQL_C_DEFAULT, SQL_C_CHAR...) */
};

/*
 * ParameterInfoClass -- stores information about a bound parameter
 */
struct ParameterInfoClass_ {
	Int4 buflen;
	char *buffer;
	Int4 *used;
	Int2 paramType;
	Int2 CType;
	Int2 SQLType;
	UInt4 precision;
	Int2 scale;
	Int4 *EXEC_used;
	char *EXEC_buffer;
	char data_at_exec;
};

BindInfoClass *create_empty_bindings(int num_columns);
void extend_bindings(StatementClass *stmt, int num_columns);

#endif
