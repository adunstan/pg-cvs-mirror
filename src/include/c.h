/*-------------------------------------------------------------------------
 *
 * c.h
 *	  Fundamental C definitions.  This is included by every .c file in
 *	  PostgreSQL (via either postgres.h or postgres_fe.h, as appropriate).
 *
 *	  Note that the definitions here are not intended to be exposed to clients
 *	  of the frontend interface libraries --- so we don't worry much about
 *	  polluting the namespace with lots of stuff...
 *
 *
 * Portions Copyright (c) 1996-2002, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $Id: c.h,v 1.144 2003/05/09 16:59:43 momjian Exp $
 *
 *-------------------------------------------------------------------------
 */
/*
 *----------------------------------------------------------------
 *	 TABLE OF CONTENTS
 *
 *		When adding stuff to this file, please try to put stuff
 *		into the relevant section, or add new sections as appropriate.
 *
 *	  section	description
 *	  -------	------------------------------------------------
 *		0)		pg_config.h and standard system headers
 *		1)		hacks to cope with non-ANSI C compilers
 *		2)		bool, true, false, TRUE, FALSE, NULL
 *		3)		standard system types
 *		4)		IsValid macros for system types
 *		5)		offsetof, lengthof, endof, alignment
 *		6)		widely useful macros
 *		7)		random stuff
 *		8)		system-specific hacks
 *
 * NOTE: since this file is included by both frontend and backend modules, it's
 * almost certainly wrong to put an "extern" declaration here.	typedefs and
 * macros are the kind of thing that might go here.
 *
 *----------------------------------------------------------------
 */
#ifndef C_H
#define C_H

/*
 * We have to include stdlib.h here because it defines many of these macros
 * on some platforms, and we only want our definitions used if stdlib.h doesn't
 * have its own.  The same goes for stddef and stdarg if present.
 */

#include "pg_config.h"
#include "pg_config_manual.h"	/* must be after pg_config.h */
#include "pg_config_os.h"
#include "postgres_ext.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdarg.h>
#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif
#include <sys/types.h>

#include <errno.h>
#include <sys/fcntl.h>			/* ensure O_BINARY is available */
#ifdef HAVE_SUPPORTDEFS_H
#include <SupportDefs.h>
#endif

/* Must be before gettext() games below */
#include <locale.h>

#ifdef ENABLE_NLS
#include <libintl.h>
#else
#define gettext(x) (x)
#endif
#define gettext_noop(x) (x)


/* ----------------------------------------------------------------
 *				Section 1: hacks to cope with non-ANSI C compilers
 *
 * type prefixes (const, signed, volatile, inline) are handled in pg_config.h.
 * ----------------------------------------------------------------
 */

/*
 * CppAsString
 *		Convert the argument to a string, using the C preprocessor.
 * CppConcat
 *		Concatenate two arguments together, using the C preprocessor.
 *
 * Note: the standard Autoconf macro AC_C_STRINGIZE actually only checks
 * whether #identifier works, but if we have that we likely have ## too.
 */
#if defined(HAVE_STRINGIZE)

#define CppAsString(identifier) #identifier
#define CppConcat(x, y)			x##y

#else							/* !HAVE_STRINGIZE */

#define CppAsString(identifier) "identifier"

/*
 * CppIdentity -- On Reiser based cpp's this is used to concatenate
 *		two tokens.  That is
 *				CppIdentity(A)B ==> AB
 *		We renamed it to _private_CppIdentity because it should not
 *		be referenced outside this file.  On other cpp's it
 *		produces  A  B.
 */
#define _priv_CppIdentity(x)x
#define CppConcat(x, y)			_priv_CppIdentity(x)y
#endif   /* !HAVE_STRINGIZE */

/*
 * dummyret is used to set return values in macros that use ?: to make
 * assignments.  gcc wants these to be void, other compilers like char
 */
#ifdef __GNUC__					/* GNU cc */
#define dummyret	void
#else
#define dummyret	char
#endif

#ifndef __GNUC__
#define __attribute__(_arg_)
#endif

/* ----------------------------------------------------------------
 *				Section 2:	bool, true, false, TRUE, FALSE, NULL
 * ----------------------------------------------------------------
 */

/*
 * bool
 *		Boolean value, either true or false.
 *
 * XXX for C++ compilers, we assume the compiler has a compatible
 * built-in definition of bool.
 */

/* BeOS defines bool already, but the compiler chokes on the
 * #ifndef unless we wrap it in this check.
 */
#ifndef __BEOS__

#ifndef __cplusplus

#ifndef bool
typedef char bool;
#endif

#ifndef true
#define true	((bool) 1)
#endif

#ifndef false
#define false	((bool) 0)
#endif
#endif   /* not C++ */
#endif   /* __BEOS__ */

typedef bool *BoolPtr;

#ifndef TRUE
#define TRUE	1
#endif

#ifndef FALSE
#define FALSE	0
#endif

/*
 * NULL
 *		Null pointer.
 */
#ifndef NULL
#define NULL	((void *) 0)
#endif


/* ----------------------------------------------------------------
 *				Section 3:	standard system types
 * ----------------------------------------------------------------
 */

/*
 * Pointer
 *		Variable holding address of any memory resident object.
 *
 *		XXX Pointer arithmetic is done with this, so it can't be void *
 *		under "true" ANSI compilers.
 */
typedef char *Pointer;

/*
 * intN
 *		Signed integer, EXACTLY N BITS IN SIZE,
 *		used for numerical computations and the
 *		frontend/backend protocol.
 */
#ifndef HAVE_INT8
typedef signed char int8;		/* == 8 bits */
typedef signed short int16;		/* == 16 bits */
typedef signed int int32;		/* == 32 bits */
#endif   /* not HAVE_INT8 */

/*
 * uintN
 *		Unsigned integer, EXACTLY N BITS IN SIZE,
 *		used for numerical computations and the
 *		frontend/backend protocol.
 */
/* Also defined in interfaces/odbc/md5.h */
#ifndef HAVE_UINT8
typedef unsigned char uint8;	/* == 8 bits */
typedef unsigned short uint16;	/* == 16 bits */
typedef unsigned int uint32;	/* == 32 bits */
#endif   /* not HAVE_UINT8 */

/*
 * boolN
 *		Boolean value, AT LEAST N BITS IN SIZE.
 */
typedef uint8 bool8;			/* >= 8 bits */
typedef uint16 bool16;			/* >= 16 bits */
typedef uint32 bool32;			/* >= 32 bits */

/*
 * bitsN
 *		Unit of bitwise operation, AT LEAST N BITS IN SIZE.
 */
typedef uint8 bits8;			/* >= 8 bits */
typedef uint16 bits16;			/* >= 16 bits */
typedef uint32 bits32;			/* >= 32 bits */

/*
 * wordN
 *		Unit of storage, AT LEAST N BITS IN SIZE,
 *		used to fetch/store data.
 */
typedef uint8 word8;			/* >= 8 bits */
typedef uint16 word16;			/* >= 16 bits */
typedef uint32 word32;			/* >= 32 bits */

/*
 * floatN
 *		Floating point number, AT LEAST N BITS IN SIZE,
 *		used for numerical computations.
 *
 *		Since sizeof(floatN) may be > sizeof(char *), always pass
 *		floatN by reference.
 *
 * XXX: these typedefs are now deprecated in favor of float4 and float8.
 * They will eventually go away.
 */
typedef float float32data;
typedef double float64data;
typedef float *float32;
typedef double *float64;

/*
 * 64-bit integers
 */
#ifdef HAVE_LONG_INT_64
/* Plain "long int" fits, use it */

#ifndef HAVE_INT64
typedef long int int64;
#endif
#ifndef HAVE_UINT64
typedef unsigned long int uint64;
#endif

#elif defined(HAVE_LONG_LONG_INT_64)
/* We have working support for "long long int", use that */

#ifndef HAVE_INT64
typedef long long int int64;
#endif
#ifndef HAVE_UINT64
typedef unsigned long long int uint64;
#endif

#else							/* not HAVE_LONG_INT_64 and not
								 * HAVE_LONG_LONG_INT_64 */

/* Won't actually work, but fall back to long int so that code compiles */
#ifndef HAVE_INT64
typedef long int int64;
#endif
#ifndef HAVE_UINT64
typedef unsigned long int uint64;
#endif

#define INT64_IS_BUSTED
#endif   /* not HAVE_LONG_INT_64 and not
								 * HAVE_LONG_LONG_INT_64 */

/* Decide if we need to decorate 64-bit constants */
#ifdef HAVE_LL_CONSTANTS
#define INT64CONST(x)  ((int64) x##LL)
#define UINT64CONST(x) ((uint64) x##LL)
#else
#define INT64CONST(x)  ((int64) x)
#define UINT64CONST(x) ((uint64) x)
#endif


/* Select timestamp representation (float8 or int64) */
#if defined(USE_INTEGER_DATETIMES) && !defined(INT64_IS_BUSTED)
#define HAVE_INT64_TIMESTAMP
#endif

/* Global variable holding time zone information. */
#ifndef HAVE_UNDERSCORE_TIMEZONE
#define TIMEZONE_GLOBAL timezone
#else
#define TIMEZONE_GLOBAL _timezone
#define tzname _tzname			/* should be in time.h? */
#endif

/* sig_atomic_t is required by ANSI C, but may be missing on old platforms */
#ifndef HAVE_SIG_ATOMIC_T
typedef int sig_atomic_t;
#endif

/*
 * Size
 *		Size of any memory resident object, as returned by sizeof.
 */
typedef size_t Size;

/*
 * Index
 *		Index into any memory resident array.
 *
 * Note:
 *		Indices are non negative.
 */
typedef unsigned int Index;

/*
 * Offset
 *		Offset into any memory resident array.
 *
 * Note:
 *		This differs from an Index in that an Index is always
 *		non negative, whereas Offset may be negative.
 */
typedef signed int Offset;

/*
 * Common Postgres datatype names (as used in the catalogs)
 */
typedef int16 int2;
typedef int32 int4;
typedef float float4;
typedef double float8;

/*
 * Oid, RegProcedure, TransactionId, CommandId, AclId
 */

/* typedef Oid is in postgres_ext.h */

/*
 * regproc is the type name used in the include/catalog headers, but
 * RegProcedure is the preferred name in C code.
 */
typedef Oid regproc;
typedef regproc RegProcedure;

typedef uint32 TransactionId;

typedef uint32 CommandId;

#define FirstCommandId	((CommandId) 0)

typedef int32 AclId;			/* user and group identifiers */

/*
 * Array indexing support
 */
#define MAXDIM 6
typedef struct
{
	int			indx[MAXDIM];
} IntArray;

/* ----------------
 *		Variable-length datatypes all share the 'struct varlena' header.
 *
 * NOTE: for TOASTable types, this is an oversimplification, since the value
 * may be compressed or moved out-of-line.	However datatype-specific routines
 * are mostly content to deal with de-TOASTed values only, and of course
 * client-side routines should never see a TOASTed value.  See postgres.h for
 * details of the TOASTed form.
 * ----------------
 */
struct varlena
{
	int32		vl_len;
	char		vl_dat[1];
};

#define VARHDRSZ		((int32) sizeof(int32))

/*
 * These widely-used datatypes are just a varlena header and the data bytes.
 * There is no terminating null or anything like that --- the data length is
 * always VARSIZE(ptr) - VARHDRSZ.
 */
typedef struct varlena bytea;
typedef struct varlena text;
typedef struct varlena BpChar;	/* blank-padded char, ie SQL char(n) */
typedef struct varlena VarChar; /* var-length char, ie SQL varchar(n) */

/*
 * Fixed-length array types (these are not varlena's!)
 */

typedef int2 int2vector[INDEX_MAX_KEYS];
typedef Oid oidvector[INDEX_MAX_KEYS];

/*
 * We want NameData to have length NAMEDATALEN and int alignment,
 * because that's how the data type 'name' is defined in pg_type.
 * Use a union to make sure the compiler agrees.  Note that NAMEDATALEN
 * must be a multiple of sizeof(int), else sizeof(NameData) will probably
 * not come out equal to NAMEDATALEN.
 */
typedef union nameData
{
	char		data[NAMEDATALEN];
	int			alignmentDummy;
} NameData;
typedef NameData *Name;

#define NameStr(name)	((name).data)


/* ----------------------------------------------------------------
 *				Section 4:	IsValid macros for system types
 * ----------------------------------------------------------------
 */
/*
 * BoolIsValid
 *		True iff bool is valid.
 */
#define BoolIsValid(boolean)	((boolean) == false || (boolean) == true)

/*
 * PointerIsValid
 *		True iff pointer is valid.
 */
#define PointerIsValid(pointer) ((void*)(pointer) != NULL)

/*
 * PointerIsAligned
 *		True iff pointer is properly aligned to point to the given type.
 */
#define PointerIsAligned(pointer, type) \
		(((long)(pointer) % (sizeof (type))) == 0)

#define OidIsValid(objectId)  ((bool) ((objectId) != InvalidOid))

#define AclIdIsValid(aclId)  ((bool) ((aclId) != 0))

#define RegProcedureIsValid(p)	OidIsValid(p)


/* ----------------------------------------------------------------
 *				Section 5:	offsetof, lengthof, endof, alignment
 * ----------------------------------------------------------------
 */
/*
 * offsetof
 *		Offset of a structure/union field within that structure/union.
 *
 *		XXX This is supposed to be part of stddef.h, but isn't on
 *		some systems (like SunOS 4).
 */
#ifndef offsetof
#define offsetof(type, field)	((long) &((type *)0)->field)
#endif   /* offsetof */

/*
 * lengthof
 *		Number of elements in an array.
 */
#define lengthof(array) (sizeof (array) / sizeof ((array)[0]))

/*
 * endof
 *		Address of the element one past the last in an array.
 */
#define endof(array)	(&array[lengthof(array)])

/* ----------------
 * Alignment macros: align a length or address appropriately for a given type.
 *
 * There used to be some incredibly crufty platform-dependent hackery here,
 * but now we rely on the configure script to get the info for us. Much nicer.
 *
 * NOTE: TYPEALIGN will not work if ALIGNVAL is not a power of 2.
 * That case seems extremely unlikely to occur in practice, however.
 * ----------------
 */

#define TYPEALIGN(ALIGNVAL,LEN) (((long)(LEN) + (ALIGNVAL-1)) & ~(ALIGNVAL-1))

#define SHORTALIGN(LEN)			TYPEALIGN(ALIGNOF_SHORT, (LEN))
#define INTALIGN(LEN)			TYPEALIGN(ALIGNOF_INT, (LEN))
#define LONGALIGN(LEN)			TYPEALIGN(ALIGNOF_LONG, (LEN))
#define DOUBLEALIGN(LEN)		TYPEALIGN(ALIGNOF_DOUBLE, (LEN))
#define MAXALIGN(LEN)			TYPEALIGN(MAXIMUM_ALIGNOF, (LEN))


/* ----------------------------------------------------------------
 *				Section 6:	widely useful macros
 * ----------------------------------------------------------------
 */
/*
 * Max
 *		Return the maximum of two numbers.
 */
#define Max(x, y)		((x) > (y) ? (x) : (y))

/*
 * Min
 *		Return the minimum of two numbers.
 */
#define Min(x, y)		((x) < (y) ? (x) : (y))

/*
 * Abs
 *		Return the absolute value of the argument.
 */
#define Abs(x)			((x) >= 0 ? (x) : -(x))

/*
 * StrNCpy
 *	Like standard library function strncpy(), except that result string
 *	is guaranteed to be null-terminated --- that is, at most N-1 bytes
 *	of the source string will be kept.
 *	Also, the macro returns no result (too hard to do that without
 *	evaluating the arguments multiple times, which seems worse).
 *
 *	BTW: when you need to copy a non-null-terminated string (like a text
 *	datum) and add a null, do not do it with StrNCpy(..., len+1).  That
 *	might seem to work, but it fetches one byte more than there is in the
 *	text object.  One fine day you'll have a SIGSEGV because there isn't
 *	another byte before the end of memory.	Don't laugh, we've had real
 *	live bug reports from real live users over exactly this mistake.
 *	Do it honestly with "memcpy(dst,src,len); dst[len] = '\0';", instead.
 */
#define StrNCpy(dst,src,len) \
	do \
	{ \
		char * _dst = (dst); \
		Size _len = (len); \
\
		if (_len > 0) \
		{ \
			strncpy(_dst, (src), _len); \
			_dst[_len-1] = '\0'; \
		} \
	} while (0)


/* Get a bit mask of the bits set in non-int32 aligned addresses */
#define INT_ALIGN_MASK (sizeof(int32) - 1)

/*
 * MemSet
 *	Exactly the same as standard library function memset(), but considerably
 *	faster for zeroing small word-aligned structures (such as parsetree nodes).
 *	This has to be a macro because the main point is to avoid function-call
 *	overhead.   However, we have also found that the loop is faster than
 *	native libc memset() on some platforms, even those with assembler
 *	memset() functions.  More research needs to be done, perhaps with
 *	platform-specific MEMSET_LOOP_LIMIT values or tests in configure.
 *
 *	bjm 2002-10-08
 */
#define MemSet(start, val, len) \
	do \
	{ \
		int32 * _start = (int32 *) (start); \
		int		_val = (val); \
		Size	_len = (len); \
\
		if ((((long) _start) & INT_ALIGN_MASK) == 0 && \
			(_len & INT_ALIGN_MASK) == 0 && \
			_val == 0 && \
			_len <= MEMSET_LOOP_LIMIT) \
		{ \
			int32 * _stop = (int32 *) ((char *) _start + _len); \
			while (_start < _stop) \
				*_start++ = 0; \
		} \
		else \
			memset((char *) _start, _val, _len); \
	} while (0)

#define MEMSET_LOOP_LIMIT  1024

/*
 * MemSetAligned is the same as MemSet except it omits the test to see if
 * "start" is word-aligned.  This is okay to use if the caller knows a-priori
 * that the pointer is suitably aligned (typically, because he just got it
 * from palloc(), which always delivers a max-aligned pointer).
 */
#define MemSetAligned(start, val, len) \
	do \
	{ \
		int32 * _start = (int32 *) (start); \
		int		_val = (val); \
		Size	_len = (len); \
\
		if ((_len & INT_ALIGN_MASK) == 0 && \
			_val == 0 && \
			_len <= MEMSET_LOOP_LIMIT) \
		{ \
			int32 * _stop = (int32 *) ((char *) _start + _len); \
			while (_start < _stop) \
				*_start++ = 0; \
		} \
		else \
			memset((char *) _start, _val, _len); \
	} while (0)


/*
 * MemSetTest/MemSetLoop are a variant version that allow all the tests in
 * MemSet to be done at compile time in cases where "val" and "len" are
 * constants *and* we know the "start" pointer must be word-aligned.
 * If MemSetTest succeeds, then it is okay to use MemSetLoop, otherwise use
 * MemSetAligned.  Beware of multiple evaluations of the arguments when using
 * this approach.
 */
#define MemSetTest(val, len) \
	( ((len) & INT_ALIGN_MASK) == 0 && \
	(len) <= MEMSET_LOOP_LIMIT && \
	(val) == 0 )

#define MemSetLoop(start, val, len) \
	do \
	{ \
		int32 * _start = (int32 *) (start); \
		int32 * _stop = (int32 *) ((char *) _start + (Size) (len)); \
	\
		while (_start < _stop) \
			*_start++ = 0; \
	} while (0)


/* ----------------------------------------------------------------
 *				Section 7:	random stuff
 * ----------------------------------------------------------------
 */

/* msb for char */
#define CSIGNBIT (0x80)

#define STATUS_OK				(0)
#define STATUS_ERROR			(-1)
#define STATUS_EOF				(-2)
#define STATUS_FOUND			(1)


/* ----------------------------------------------------------------
 *				Section 8: system-specific hacks
 *
 *		This should be limited to things that absolutely have to be
 *		included in every source file.	The port-specific header file
 *		is usually a better place for this sort of thing.
 * ----------------------------------------------------------------
 */

#if defined(__CYGWIN__) || defined(WIN32)
#define PG_BINARY	O_BINARY
#define PG_BINARY_R "rb"
#define PG_BINARY_W "wb"
#else
#define PG_BINARY	0
#define PG_BINARY_R "r"
#define PG_BINARY_W "w"
#endif

#if defined(sun) && defined(__sparc__) && !defined(__SVR4)
#include <unistd.h>
#endif

/* Portable path handling for Unix/Win32 */
bool is_absolute_path(const char *filename);
char *first_path_separator(const char *filename);
char *last_path_separator(const char *filename);
char *get_progname(char *argv0);

#if defined(bsdi) || defined(netbsd)
int fseeko(FILE *stream, off_t offset, int whence);
off_t ftello(FILE *stream);
#endif

/*
 * Win32 doesn't have reliable rename/unlink during concurrent access
 */
#if defined(WIN32) && !defined(FRONTEND)
int pgrename(const char *from, const char *to);
int pgunlink(const char *path);      
#define rename(path)		pgrename(path)
#define unlink(from, to)	pgunlink(from, to)
#endif

/*
 * Win32 doesn't have opendir/readdir/closedir()
 */
#ifdef WIN32
struct dirent {
	ino_t d_ino;					/* inode (always 1 on WIN32) */
	char d_name[MAX_PATH + 1];	/* filename (null terminated) */
};

typedef struct {
	HANDLE handle;				/* handle for FindFirstFile or
								 * FindNextFile */
	long offset;				/* offset into directory */
	int finished;				/* 1 if there are not more files */
	WIN32_FIND_DATA finddata;	/* file data FindFirstFile or FindNextFile
								 * returns */
	char *dir;					/* the directory path we are reading */
	struct dirent ent;			/* the dirent to return */
} DIR;

extern DIR *opendir(const char *);
extern struct dirent *readdir(DIR *);
extern int closedir(DIR *);
#endif

/*
 *	Win32 requires a special close for sockets and pipes, while on Unix
 *	close() does them all.
 */
#ifndef WIN32
#define	closesocket close
#endif
  
/* These are for things that are one way on Unix and another on NT */
#define NULL_DEV		"/dev/null"

/*
 * Provide prototypes for routines not present in a particular machine's
 * standard C library.
 */

#if !HAVE_DECL_SNPRINTF
extern int
snprintf(char *str, size_t count, const char *fmt,...)
/* This extension allows gcc to check the format string */
__attribute__((format(printf, 3, 4)));
#endif

#if !HAVE_DECL_VSNPRINTF
extern int	vsnprintf(char *str, size_t count, const char *fmt, va_list args);
#endif

#if !defined(HAVE_MEMMOVE) && !defined(memmove)
#define memmove(d, s, c)		bcopy(s, d, c)
#endif

#ifndef DLLIMPORT
#define DLLIMPORT				/* no special DLL markers on most ports */
#endif

/*
 * The following is used as the arg list for signal handlers.  Any ports
 * that take something other than an int argument should override this in
 * their pg_config_os.h file.  Note that variable names are required
 * because it is used in both the prototypes as well as the definitions.
 * Note also the long name.  We expect that this won't collide with
 * other names causing compiler warnings.
 */ 

#ifndef SIGNAL_ARGS
#define SIGNAL_ARGS  int postgres_signal_arg
#endif

/*
 * Default "extern" declarations or macro substitutes for library routines.
 * When necessary, these routines are provided by files in src/port/.
 */
#ifndef HAVE_CRYPT
char *crypt(const char *key, const char *setting);
#endif

#ifndef HAVE_FSEEKO
#define fseeko(a, b, c) fseek((a), (b), (c))
#define ftello(a) ftell((a))
#endif

#ifndef HAVE_ISINF
extern int isinf(double x);
#endif

#ifndef HAVE_GETHOSTNAME
extern int gethostname(char *name, int namelen);
#endif

#ifndef HAVE_RINT
extern double rint(double x);
#endif

#ifndef HAVE_INET_ATON
# include <netinet/in.h>
# include <arpa/inet.h>
extern int inet_aton(const char *cp, struct in_addr * addr);
#endif

/*
 * When there is no sigsetjmp, its functionality is provided by plain
 * setjmp. Incidentally, nothing provides setjmp's functionality in
 * that case.
 */
#ifndef HAVE_SIGSETJMP
# define sigjmp_buf jmp_buf
# define sigsetjmp(x,y)	setjmp(x)
# define siglongjmp longjmp
#endif

#ifndef HAVE_STRCASECMP
extern int strcasecmp(char *s1, char *s2);
#endif

#ifndef HAVE_STRDUP
extern char *strdup(char const *);
#endif

#ifndef HAVE_RANDOM
extern long random(void);
#endif

#ifndef HAVE_SRANDOM
extern void srandom(unsigned int seed);
#endif

#if defined(HAVE_FDATASYNC) && !HAVE_DECL_FDATASYNC
extern int fdatasync(int fildes);
#endif

/* If strtoq() exists, rename it to the more standard strtoll() */
#if defined(HAVE_LONG_LONG_INT_64) && !defined(HAVE_STRTOLL) && defined(HAVE_STRTOQ)
# define strtoll strtoq
# define HAVE_STRTOLL 1
#endif

/* If strtouq() exists, rename it to the more standard strtoull() */
#if defined(HAVE_LONG_LONG_INT_64) && !defined(HAVE_STRTOULL) && defined(HAVE_STRTOUQ)
# define strtoull strtouq
# define HAVE_STRTOULL 1
#endif

#endif   /* C_H */
