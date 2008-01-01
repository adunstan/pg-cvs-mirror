/*
 * psql - the PostgreSQL interactive terminal
 *
 * Copyright (c) 2000-2008, PostgreSQL Global Development Group
 *
 * $PostgreSQL: pgsql/src/bin/psql/prompt.h,v 1.18 2007/01/05 22:19:49 momjian Exp $
 */
#ifndef PROMPT_H
#define PROMPT_H

typedef enum _promptStatus
{
	PROMPT_READY,
	PROMPT_CONTINUE,
	PROMPT_COMMENT,
	PROMPT_SINGLEQUOTE,
	PROMPT_DOUBLEQUOTE,
	PROMPT_DOLLARQUOTE,
	PROMPT_PAREN,
	PROMPT_COPY
} promptStatus_t;

char	   *get_prompt(promptStatus_t status);

#endif   /* PROMPT_H */
