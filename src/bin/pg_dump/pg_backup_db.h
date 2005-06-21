/*
 *	Definitions for pg_backup_db.c
 *
 *	IDENTIFICATION
 *		$PostgreSQL: pgsql/src/bin/pg_dump/pg_backup_db.h,v 1.10 2004/03/03 21:28:54 tgl Exp $
 */

extern int	ExecuteSqlCommand(ArchiveHandle *AH, PQExpBuffer qry, char *desc);
extern int	ExecuteSqlCommandBuf(ArchiveHandle *AH, void *qry, size_t bufLen);

extern void StartTransaction(ArchiveHandle *AH);
extern void CommitTransaction(ArchiveHandle *AH);
