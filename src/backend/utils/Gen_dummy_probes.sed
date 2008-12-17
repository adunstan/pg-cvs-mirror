#-------------------------------------------------------------------------
# sed script to create dummy probes.h file when dtrace is not available
#
# Copyright (c) 2008, PostgreSQL Global Development Group
#
# $PostgreSQL: pgsql/src/backend/utils/Gen_dummy_probes.sed,v 1.2 2008/08/01 13:16:09 alvherre Exp $
#-------------------------------------------------------------------------

/^[ 	]*probe /!d
s/^[ 	]*probe \([^(]*\)\(.*\);/\1\2/
s/__/_/g
y/abcdefghijklmnopqrstuvwxyz/ABCDEFGHIJKLMNOPQRSTUVWXYZ/
s/^/#define TRACE_POSTGRESQL_/
s/([^,)]\{1,\})/(INT1)/
s/([^,)]\{1,\}, [^,)]\{1,\})/(INT1, INT2)/
s/([^,)]\{1,\}, [^,)]\{1,\}, [^,)]\{1,\})/(INT1, INT2, INT3)/
s/([^,)]\{1,\}, [^,)]\{1,\}, [^,)]\{1,\}, [^,)]\{1,\})/(INT1, INT2, INT3, INT4)/
s/([^,)]\{1,\}, [^,)]\{1,\}, [^,)]\{1,\}, [^,)]\{1,\}, [^,)]\{1,\})/(INT1, INT2, INT3, INT4, INT5)/
s/([^,)]\{1,\}, [^,)]\{1,\}, [^,)]\{1,\}, [^,)]\{1,\}, [^,)]\{1,\}, [^,)]\{1,\})/(INT1, INT2, INT3, INT4, INT5, INT6)/
s/([^,)]\{1,\}, [^,)]\{1,\}, [^,)]\{1,\}, [^,)]\{1,\}, [^,)]\{1,\}, [^,)]\{1,\}, [^,)]\{1,\})/(INT1, INT2, INT3, INT4, INT5, INT6, INT7)/
s/([^,)]\{1,\}, [^,)]\{1,\}, [^,)]\{1,\}, [^,)]\{1,\}, [^,)]\{1,\}, [^,)]\{1,\}, [^,)]\{1,\}, [^,)]\{1,\})/(INT1, INT2, INT3, INT4, INT5, INT6, INT7, INT8)/
P
s/(.*$/_ENABLED() (0)/
