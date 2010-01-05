/*
 * $PostgreSQL: pgsql/src/interfaces/ecpg/include/sqlda.h,v 1.4 2009/06/11 14:49:13 momjian Exp $
 */

#ifndef ECPG_SQLDA_H
#define ECPG_SQLDA_H

#ifdef _ECPG_INFORMIX_H

#include "sqlda-compat.h"
typedef struct sqlvar_compat	sqlvar_t;
typedef struct sqlda_compat	sqlda_t;

#else

#include "sqlda-native.h"
typedef struct sqlvar_struct	sqlvar_t;
typedef struct sqlda_struct	sqlda_t;

#endif

#endif /* ECPG_SQLDA_H */
