#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "type.h"

/* Constructors
   Yes, I mostly write c++-code
   */

/* The NAME argument is copied. The type argument is preserved as a pointer. */
struct ECPGrecord_member *
ECPGmake_record_member(char * name, struct ECPGtype * type)
{
    struct ECPGrecord_member * ne = 
	(struct ECPGrecord_member *)malloc(sizeof(struct ECPGrecord_member));

    ne->name = strdup(name);
    ne->typ = type;

    return ne;
}

struct ECPGtype *
ECPGmake_simple_type(enum ECPGttype typ)
{
    struct ECPGtype * ne = (struct ECPGtype *)malloc(sizeof(struct ECPGtype));

    ne->typ = typ;
    ne->size = 0;
    ne->u.element = 0;

    return ne;
}

struct ECPGtype *
ECPGmake_varchar_type(enum ECPGttype typ, unsigned short siz)
{
    struct ECPGtype * ne = ECPGmake_simple_type(typ);

    ne->size = siz;

    return ne;
}

struct ECPGtype *
ECPGmake_array_type(struct ECPGtype * typ, unsigned short siz)
{
    struct ECPGtype * ne = ECPGmake_simple_type(ECPGt_array);

    ne->size = siz;
    ne->u.element = typ;

    return ne;
}

struct ECPGtype *
ECPGmake_record_type(struct ECPGrecord_member * rm[])
{
    struct ECPGtype * ne = ECPGmake_simple_type(ECPGt_record);

    ne->u.members = rm;

    return ne;
}


/* Dump a type.
   The type is dumped as:
	type-tag <comma> 		- enum ECPGttype
	reference-to-variable <comma> 	- void *
	size <comma>			- short size of this field (if varchar)
	arrsize <comma>			- short number of elements in the arr
	offset <comma>			- short offset to the next element
   Where:
   type-tag is one of the simple types or varchar.
   reference-to-variable can be a reference to a struct element.
   arrsize is the size of the array in case of array fetches. Otherwise 0.
   size is the maxsize in case it is a varchar. Otherwise it is the size of
   	the variable (required to do array fetches of records).
 */
void ECPGdump_a_simple(FILE * o, const char * name, enum ECPGttype typ, 
		   short varcharsize,
		   unsigned short arrsiz, const char * siz);
void ECPGdump_a_record(FILE * o, const char * name, unsigned short arrsiz,
		   struct ECPGtype * typ, const char * offset);


void
ECPGdump_a_type(FILE * o, const char * name, struct ECPGtype * typ)
{
    if (IS_SIMPLE_TYPE(typ->typ))
    {
	ECPGdump_a_simple(o, name, typ->typ, typ->size, 0, 0);
    }
    else if (typ->typ == ECPGt_array)
    {
	if (IS_SIMPLE_TYPE(typ->u.element->typ))
	    ECPGdump_a_simple(o, name, typ->u.element->typ,
			  typ->u.element->size, typ->size, 0);
	else if (typ->u.element->typ == ECPGt_array)
	{
	    abort();		/* Array of array, */
	}
	else if (typ->u.element->typ == ECPGt_record)
	{
	    /* Array of records. */
	    ECPGdump_a_record(o, name, typ->size, typ->u.element, 0);
	}
	else
	{
	    abort();
	}
    }
    else if (typ->typ == ECPGt_record)
    {
	ECPGdump_a_record(o, name, 0, typ, 0);
    }
    else
    {
	abort();
    }
}


/* If siz is NULL, then the offset is 0, if not use siz as a
   string, it represents the offset needed if we are in an array of records. */
void
ECPGdump_a_simple(FILE * o, const char * name, enum ECPGttype typ,
		  short varcharsize,
		  unsigned short arrsiz,
		  const char * siz)
{
    switch (typ)
    {
    case ECPGt_char:
	fprintf(o, "ECPGt_char,&%s,0,%d,%s, ", name, arrsiz, 
		siz == NULL ? "sizeof(char)" : siz);
	break;
    case ECPGt_unsigned_char:
	fprintf(o, "ECPGt_unsigned_char,&%s,0,%d,%s, ", name, arrsiz,
		siz == NULL ? "sizeof(unsigned char)" : siz);
	break;
    case ECPGt_short:
	fprintf(o, "ECPGt_short,&%s,0,%d,%s, ", name, arrsiz,
		siz == NULL ? "sizeof(short)" : siz);
	break; 
    case ECPGt_unsigned_short:
	fprintf(o, 
		"ECPGt_unsigned_short,&%s,0,%d,%s, ", name, arrsiz,
		siz == NULL ? "sizeof(unsigned short)" : siz);
	break;
    case ECPGt_int:
	fprintf(o, "ECPGt_int,&%s,0,%d,%s, ", name, arrsiz,
		siz == NULL ? "sizeof(int)" : siz);
	break;
    case ECPGt_unsigned_int:
	fprintf(o, "ECPGt_unsigned_int,&%s,0,%d,%s, ", name, arrsiz,
		siz == NULL ? "sizeof(unsigned int)" : siz);
	break;
    case ECPGt_long:
	fprintf(o, "ECPGt_long,&%s,0,%d,%s, ", name, arrsiz,
		siz == NULL ? "sizeof(long)" : siz);
	break;
    case ECPGt_unsigned_long:
	fprintf(o, "ECPGt_unsigned_int,&%s,0,%d,%s, ", name, arrsiz,
		siz == NULL ? "sizeof(unsigned int)" : siz);
	break;
    case ECPGt_float:
	fprintf(o, "ECPGt_float,&%s,0,%d,%s, ", name, arrsiz,
		siz == NULL ? "sizeof(float)" : siz);
	break;
    case ECPGt_double:
	fprintf(o, "ECPGt_double,&%s,0,%d,%s, ", name, arrsiz,
		siz == NULL ? "sizeof(double)" : siz);
	break;
    case ECPGt_varchar:
    case ECPGt_varchar2:
	if (siz == NULL)
	    fprintf(o, "ECPGt_varchar,&%s,%d,%d,sizeof(struct varchar_%s), ", 
		    name,
		    varcharsize,
		    arrsiz, name);
	else
	    fprintf(o, "ECPGt_varchar,&%s,%d,%d,%s, ", 
		    name, 
		    varcharsize,
		    arrsiz, siz);
	break;
    default:
	abort();
    }
}


/* Penetrate a record and dump the contents. */
void
ECPGdump_a_record(FILE * o,
		  const char * name, unsigned short arrsiz,
		  struct ECPGtype * typ, const char * offsetarg)
{
    /* If offset is NULL, then this is the first recursive level. If not then 
       we are in a record in a record and the offset is used as offset.
     */
    struct ECPGrecord_member ** p;
    char obuf[BUFSIZ];
    char buf[BUFSIZ];
    const char * offset;

    if (offsetarg == NULL)
    {
	sprintf(obuf, "sizeof(%s)", name);
	offset = obuf;
    }
    else
    {
	offset = offsetarg;
    }

    for (p = typ->u.members; *p; p++)
    {
	if (IS_SIMPLE_TYPE((*p)->typ->typ))
	{
	    sprintf(buf, "%s.%s", name, (*p)->name);
	    ECPGdump_a_simple(o, buf, (*p)->typ->typ, (*p)->typ->size,
			  arrsiz, offset);
	}
	else if ((*p)->typ->typ == ECPGt_array)
	{
	    int i;

	    for (i = 0; i < (*p)->typ->size; i++)
	    {
		if (IS_SIMPLE_TYPE((*p)->typ->u.element->typ))
		{
		    sprintf(buf, "%s.%s[%d]", name, (*p)->name, i);
		    ECPGdump_a_simple(o, buf, (*p)->typ->typ, (*p)->typ->size,
				      arrsiz, offset);
		}
		else if((*p)->typ->u.element->typ == ECPGt_array)
		{
		    /* Array within an array. NOT implemented yet. */
		    abort();
		}
		else if ((*p)->typ->u.element->typ == ECPGt_record)
		{
		    /* Record within array within record. NOT implemented yet. */
		    abort();
		}
		else	
		{
		    /* Unknown type */
		    abort();
		}
	    }
	}
	else if ((*p)->typ->typ == ECPGt_record)
	{
	    /* Record within a record */
	    sprintf(buf, "%s.%s", name, (*p)->name);
	    ECPGdump_a_record(o, buf, arrsiz, (*p)->typ, offset);
	}
	else
	{
	    /* Unknown type */
	    abort();
	}
    }
}


/* Freeing is not really that important. Since we throw away the process
   anyway. Lets implement that last! */

void
ECPGfree_record_member(struct ECPGrecord_member * rm)
{
}

void
ECPGfree_type(struct ECPGtype * typ)
{
}
