/*-------------------------------------------------------------------------
 *
 * geqo_gene.h
 *	  genome representation in optimizer/geqo
 *
 * Portions Copyright (c) 1996-2001, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $Id: geqo_gene.h,v 1.12 2001/10/28 06:26:07 momjian Exp $
 *
 *-------------------------------------------------------------------------
 */

/* contributed by:
   =*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=
   *  Martin Utesch				 * Institute of Automatic Control	   *
   =							 = University of Mining and Technology =
   *  utesch@aut.tu-freiberg.de  * Freiberg, Germany				   *
   =*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=
 */


#ifndef GEQO_GENE_H
#define GEQO_GENE_H

#include "nodes/nodes.h"

/* we presume that int instead of Relid
   is o.k. for Gene; so don't change it! */
typedef int Gene;

typedef struct Chromosome
{
	Gene	   *string;
	Cost		worth;
} Chromosome;

typedef struct Pool
{
	Chromosome *data;
	int			size;
	int			string_length;
} Pool;

#endif	 /* GEQO_GENE_H */
