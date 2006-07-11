/*
 *	Definitions for pg_backup_db.c
 *
 *	IDENTIFICATION
 *		$PostgreSQL: pgsql/src/bin/pg_dump/pg_backup_db.h,v 1.11 2005/06/21 20:45:44 tgl Exp $
 */

#ifndef PG_BACKUP_DB_H
#define PG_BACKUP_DB_H

#include "pg_backup_archiver.h"

extern int	ExecuteSqlCommand(ArchiveHandle *AH, PQExpBuffer qry, char *desc);
extern int	ExecuteSqlCommandBuf(ArchiveHandle *AH, void *qry, size_t bufLen);

extern void StartTransaction(ArchiveHandle *AH);
extern void CommitTransaction(ArchiveHandle *AH);

#endif
