/*
 * xlog_internal.h
 *
 * PostgreSQL transaction log internal declarations
 *
 * NOTE: this file is intended to contain declarations useful for
 * manipulating the XLOG files directly, but it is not supposed to be
 * needed by rmgr routines (redo/undo support for individual record types).
 * So the XLogRecord typedef and associated stuff appear in xlog.h.
 *
 * Portions Copyright (c) 1996-2004, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $PostgreSQL: pgsql-server/src/include/access/xlog_internal.h,v 1.2 2004/08/03 20:32:34 tgl Exp $
 */
#ifndef XLOG_INTERNAL_H
#define XLOG_INTERNAL_H

#include "access/xlog.h"
#include "fmgr.h"
#include "storage/block.h"
#include "storage/relfilenode.h"


/*
 * Header info for a backup block appended to an XLOG record.
 *
 * Note that the backup block has its own CRC, and is not covered by
 * the CRC of the XLOG record proper.  Also note that we don't attempt
 * to align either the BkpBlock struct or the block's data.
 */
typedef struct BkpBlock
{
	crc64		crc;
	RelFileNode node;
	BlockNumber block;
} BkpBlock;

/*
 * When there is not enough space on current page for whole record, we
 * continue on the next page with continuation record.	(However, the
 * XLogRecord header will never be split across pages; if there's less than
 * SizeOfXLogRecord space left at the end of a page, we just waste it.)
 *
 * Note that xl_rem_len includes backup-block data, unlike xl_len in the
 * initial header.
 */
typedef struct XLogContRecord
{
	uint32		xl_rem_len;		/* total len of remaining data for record */

	/* ACTUAL LOG DATA FOLLOWS AT END OF STRUCT */

} XLogContRecord;

#define SizeOfXLogContRecord	MAXALIGN(sizeof(XLogContRecord))

/*
 * Each page of XLOG file has a header like this:
 */
#define XLOG_PAGE_MAGIC 0xD05B	/* can be used as WAL version indicator */

typedef struct XLogPageHeaderData
{
	uint16		xlp_magic;		/* magic value for correctness checks */
	uint16		xlp_info;		/* flag bits, see below */
	TimeLineID	xlp_tli;		/* TimeLineID of first record on page */
	XLogRecPtr	xlp_pageaddr;	/* XLOG address of this page */
} XLogPageHeaderData;

#define SizeOfXLogShortPHD	MAXALIGN(sizeof(XLogPageHeaderData))

typedef XLogPageHeaderData *XLogPageHeader;

/*
 * When the XLP_LONG_HEADER flag is set, we store additional fields in the
 * page header.  (This is ordinarily done just in the first page of an
 * XLOG file.)  The additional fields serve to identify the file accurately.
 */
typedef struct XLogLongPageHeaderData
{
	XLogPageHeaderData std;		/* standard header fields */
	uint64		xlp_sysid;		/* system identifier from pg_control */
	uint32		xlp_seg_size;	/* just as a cross-check */
} XLogLongPageHeaderData;

#define SizeOfXLogLongPHD	MAXALIGN(sizeof(XLogLongPageHeaderData))

typedef XLogLongPageHeaderData *XLogLongPageHeader;

/* When record crosses page boundary, set this flag in new page's header */
#define XLP_FIRST_IS_CONTRECORD		0x0001
/* This flag indicates a "long" page header */
#define XLP_LONG_HEADER				0x0002
/* All defined flag bits in xlp_info (used for validity checking of header) */
#define XLP_ALL_FLAGS				0x0003

#define XLogPageHeaderSize(hdr)		\
	(((hdr)->xlp_info & XLP_LONG_HEADER) ? SizeOfXLogLongPHD : SizeOfXLogShortPHD)

/*
 * We break each logical log file (xlogid value) into segment files of the
 * size indicated by XLOG_SEG_SIZE.  One possible segment at the end of each
 * log file is wasted, to ensure that we don't have problems representing
 * last-byte-position-plus-1.
 */
#define XLogSegSize		((uint32) XLOG_SEG_SIZE)
#define XLogSegsPerFile (((uint32) 0xffffffff) / XLogSegSize)
#define XLogFileSize	(XLogSegsPerFile * XLogSegSize)


/*
 * Macros for manipulating XLOG pointers
 */

/* Increment an xlogid/segment pair */
#define NextLogSeg(logId, logSeg)	\
	do { \
		if ((logSeg) >= XLogSegsPerFile-1) \
		{ \
			(logId)++; \
			(logSeg) = 0; \
		} \
		else \
			(logSeg)++; \
	} while (0)

/* Decrement an xlogid/segment pair (assume it's not 0,0) */
#define PrevLogSeg(logId, logSeg)	\
	do { \
		if (logSeg) \
			(logSeg)--; \
		else \
		{ \
			(logId)--; \
			(logSeg) = XLogSegsPerFile-1; \
		} \
	} while (0)

/*
 * Compute ID and segment from an XLogRecPtr.
 *
 * For XLByteToSeg, do the computation at face value.  For XLByteToPrevSeg,
 * a boundary byte is taken to be in the previous segment.	This is suitable
 * for deciding which segment to write given a pointer to a record end,
 * for example.  (We can assume xrecoff is not zero, since no valid recptr
 * can have that.)
 */
#define XLByteToSeg(xlrp, logId, logSeg)	\
	( logId = (xlrp).xlogid, \
	  logSeg = (xlrp).xrecoff / XLogSegSize \
	)
#define XLByteToPrevSeg(xlrp, logId, logSeg)	\
	( logId = (xlrp).xlogid, \
	  logSeg = ((xlrp).xrecoff - 1) / XLogSegSize \
	)

/*
 * Is an XLogRecPtr within a particular XLOG segment?
 *
 * For XLByteInSeg, do the computation at face value.  For XLByteInPrevSeg,
 * a boundary byte is taken to be in the previous segment.
 */
#define XLByteInSeg(xlrp, logId, logSeg)	\
	((xlrp).xlogid == (logId) && \
	 (xlrp).xrecoff / XLogSegSize == (logSeg))

#define XLByteInPrevSeg(xlrp, logId, logSeg)	\
	((xlrp).xlogid == (logId) && \
	 ((xlrp).xrecoff - 1) / XLogSegSize == (logSeg))

/* Check if an xrecoff value is in a plausible range */
#define XRecOffIsValid(xrecoff) \
		((xrecoff) % BLCKSZ >= SizeOfXLogShortPHD && \
		(BLCKSZ - (xrecoff) % BLCKSZ) >= SizeOfXLogRecord)

/*
 * These macros encapsulate knowledge about the exact layout of XLog file
 * names, timeline history file names, and archive-status file names.
 */
#define MAXFNAMELEN		64

#define XLogFileName(fname, tli, log, seg)	\
	snprintf(fname, MAXFNAMELEN, "%08X%08X%08X", tli, log, seg)

#define XLogFilePath(path, tli, log, seg)	\
	snprintf(path, MAXPGPATH, "%s/%08X%08X%08X", XLogDir, tli, log, seg)

#define TLHistoryFileName(fname, tli)	\
	snprintf(fname, MAXFNAMELEN, "%08X.history", tli)

#define TLHistoryFilePath(path, tli)	\
	snprintf(path, MAXPGPATH, "%s/%08X.history", XLogDir, tli)

#define StatusFilePath(path, xlog, suffix)	\
	snprintf(path, MAXPGPATH, "%s/archive_status/%s%s", XLogDir, xlog, suffix)

#define BackupHistoryFileName(fname, tli, log, seg, offset)	\
	snprintf(fname, MAXFNAMELEN, "%08X%08X%08X.%08X.backup", tli, log, seg, offset)

#define BackupHistoryFilePath(path, tli, log, seg, offset)	\
	snprintf(path, MAXPGPATH, "%s/%08X%08X%08X.%08X.backup", XLogDir, tli, log, seg, offset)

extern char XLogDir[MAXPGPATH];

/*
 * _INTL_MAXLOGRECSZ: max space needed for a record including header and
 * any backup-block data.
 */
#define _INTL_MAXLOGRECSZ	(SizeOfXLogRecord + MAXLOGRECSZ + \
							 XLR_MAX_BKP_BLOCKS * (sizeof(BkpBlock) + BLCKSZ))


/*
 * Method table for resource managers.
 *
 * RmgrTable[] is indexed by RmgrId values (see rmgr.h).
 */
typedef struct RmgrData
{
	const char *rm_name;
	void		(*rm_redo) (XLogRecPtr lsn, XLogRecord *rptr);
	void		(*rm_undo) (XLogRecPtr lsn, XLogRecord *rptr);
	void		(*rm_desc) (char *buf, uint8 xl_info, char *rec);
	void		(*rm_startup) (void);
	void		(*rm_cleanup) (void);
} RmgrData;

extern const RmgrData RmgrTable[];

/*
 * These aren't in xlog.h because I'd rather not include fmgr.h there.
 */
extern Datum pg_start_backup(PG_FUNCTION_ARGS);
extern Datum pg_stop_backup(PG_FUNCTION_ARGS);

#endif   /* XLOG_INTERNAL_H */
