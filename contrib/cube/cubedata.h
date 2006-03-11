/* $PostgreSQL: pgsql/src/backend/utils/misc/guc.c,v 1.314 2006/03/07 02:54:23 momjian Exp $ */

#define CUBE_MAX_DIM (100)
typedef struct NDBOX
{
	unsigned int size;			/* required to be a Postgres varlena type */
	unsigned int dim;
	double		x[1];
}	NDBOX;
