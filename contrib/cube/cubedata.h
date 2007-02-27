/* $PostgreSQL: pgsql/contrib/cube/cubedata.h,v 1.7 2006/03/11 04:38:28 momjian Exp $ */

#define CUBE_MAX_DIM (100)

typedef struct NDBOX
{
	int32		vl_len_;		/* varlena header (do not touch directly!) */
	unsigned int dim;
	double		x[1];
}	NDBOX;
