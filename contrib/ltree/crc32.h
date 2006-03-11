#ifndef _CRC32_H
#define _CRC32_H

/* $PostgreSQL: pgsql/src/backend/utils/misc/guc.c,v 1.314 2006/03/07 02:54:23 momjian Exp $ */

/* Returns crc32 of data block */
extern unsigned int ltree_crc32_sz(char *buf, int size);

/* Returns crc32 of null-terminated string */
#define crc32(buf) ltree_crc32_sz((buf),strlen(buf))

#endif
