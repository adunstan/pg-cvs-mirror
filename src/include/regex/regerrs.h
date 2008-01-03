/*
 * $PostgreSQL: pgsql/src/include/regex/regerrs.h,v 1.4 2007/02/01 19:10:29 momjian Exp $
 */

{
	REG_OKAY, "REG_OKAY", "no errors detected"
},

{
	REG_NOMATCH, "REG_NOMATCH", "failed to match"
},

{
	REG_BADPAT, "REG_BADPAT", "invalid regexp (reg version 0.8)"
},

{
	REG_ECOLLATE, "REG_ECOLLATE", "invalid collating element"
},

{
	REG_ECTYPE, "REG_ECTYPE", "invalid character class"
},

{
	REG_EESCAPE, "REG_EESCAPE", "invalid escape \\ sequence"
},

{
	REG_ESUBREG, "REG_ESUBREG", "invalid backreference number"
},

{
	REG_EBRACK, "REG_EBRACK", "brackets [] not balanced"
},

{
	REG_EPAREN, "REG_EPAREN", "parentheses () not balanced"
},

{
	REG_EBRACE, "REG_EBRACE", "braces {} not balanced"
},

{
	REG_BADBR, "REG_BADBR", "invalid repetition count(s)"
},

{
	REG_ERANGE, "REG_ERANGE", "invalid character range"
},

{
	REG_ESPACE, "REG_ESPACE", "out of memory"
},

{
	REG_BADRPT, "REG_BADRPT", "quantifier operand invalid"
},

{
	REG_ASSERT, "REG_ASSERT", "\"cannot happen\" -- you found a bug"
},

{
	REG_INVARG, "REG_INVARG", "invalid argument to regex function"
},

{
	REG_MIXED, "REG_MIXED", "character widths of regex and string differ"
},

{
	REG_BADOPT, "REG_BADOPT", "invalid embedded option"
},

{
	REG_ETOOBIG, "REG_ETOOBIG", "nfa has too many states"
},
