--
-- Sanity checks for common errors in making pg_operator table.
-- None of the SELECTs here should ever find any matching entries,
-- so the expected output is easy to maintain ;-).
-- A test failure indicates someone messed up an entry in pg_operator.h.
--
-- NB: run this test earlier than the create_operator test, because
-- that test creates some bogus operators...
--

-- Look for bogus data types.

SELECT p1.oid, p1.* FROM pg_operator AS p1
WHERE p1.oprleft != 0 AND NOT EXISTS(SELECT * FROM pg_type AS t1 WHERE t1.oid = p1.oprleft);

SELECT p1.oid, p1.* FROM pg_operator AS p1
WHERE p1.oprright != 0 AND NOT EXISTS(SELECT * FROM pg_type AS t1 WHERE t1.oid = p1.oprright);

SELECT p1.oid, p1.* FROM pg_operator AS p1
WHERE p1.oprresult != 0 AND NOT EXISTS(SELECT * FROM pg_type AS t1 WHERE t1.oid = p1.oprresult);

-- Look for dangling links to other operators.

SELECT p1.oid, p1.* FROM pg_operator AS p1
WHERE p1.oprcom != 0 AND NOT
      EXISTS(SELECT * FROM pg_operator AS p2 WHERE p2.oid = p1.oprcom);

SELECT p1.oid, p1.* FROM pg_operator AS p1
WHERE p1.oprnegate != 0 AND NOT
      EXISTS(SELECT * FROM pg_operator AS p2 WHERE p2.oid = p1.oprnegate);

SELECT p1.oid, p1.* FROM pg_operator AS p1
WHERE p1.oprlsortop != 0 AND NOT
      EXISTS(SELECT * FROM pg_operator AS p2 WHERE p2.oid = p1.oprlsortop);

SELECT p1.oid, p1.* FROM pg_operator AS p1
WHERE p1.oprrsortop != 0 AND NOT
      EXISTS(SELECT * FROM pg_operator AS p2 WHERE p2.oid = p1.oprrsortop);

-- FIXME: how can we test for a dangling OPRCODE value?

-- Look for conflicting operator definitions (same names and input datatypes).

SELECT p1.oid, p1.oprcode, p2.oid, p2.oprcode
FROM pg_operator AS p1, pg_operator AS p2
WHERE p1.oid != p2.oid AND
    p1.oprname = p2.oprname AND
    p1.oprkind = p2.oprkind AND
    p1.oprleft = p2.oprleft AND
    p1.oprright = p2.oprright;

-- Look for commutative operators that don't commute.
-- DEFINITIONAL NOTE: If A.oprcom = B, then x A y has the same result as y B x.
-- We expect that B will always say that B.oprcom = A as well; that's not
-- inherently essential, but it would be inefficient not to mark it so.

SELECT p1.oid, p1.oprcode, p2.oid, p2.oprcode
FROM pg_operator AS p1, pg_operator AS p2
WHERE p1.oprcom = p2.oid AND
    (p1.oprkind != 'b' OR
     p1.oprleft != p2.oprright OR
     p1.oprright != p2.oprleft OR
     p1.oprresult != p2.oprresult OR
     p1.oid != p2.oprcom);

-- Look for negatory operators that don't agree.
-- DEFINITIONAL NOTE: If A.oprnegate = B, then both A and B must yield
-- boolean results, and (x A y) == ! (x B y), or the equivalent for
-- single-operand operators.
-- We expect that B will always say that B.oprnegate = A as well; that's not
-- inherently essential, but it would be inefficient not to mark it so.
-- NOTE hardwired assumption that type bool has OID 16.

SELECT p1.oid, p1.oprcode, p2.oid, p2.oprcode
FROM pg_operator AS p1, pg_operator AS p2
WHERE p1.oprnegate = p2.oid AND
    (p1.oprkind != p2.oprkind OR
     p1.oprleft != p2.oprleft OR
     p1.oprright != p2.oprright OR
     p1.oprresult != 16 OR
     p2.oprresult != 16 OR
     p1.oid != p2.oprnegate);

-- Look for mergejoin operators that don't match their links.
-- A mergejoin link leads from an '=' operator to the
-- sort operator ('<' operator) that's appropriate for
-- its left-side or right-side data type.

SELECT p1.oid, p1.oprcode, p2.oid, p2.oprcode
FROM pg_operator AS p1, pg_operator AS p2
WHERE p1.oprlsortop = p2.oid AND
    (p1.oprname != '=' OR p2.oprname != '<' OR
     p1.oprkind != 'b' OR p2.oprkind != 'b' OR
     p1.oprleft != p2.oprleft OR
     p1.oprleft != p2.oprright OR
     p1.oprresult != 16 OR
     p2.oprresult != 16 OR
     p1.oprrsortop = 0);

SELECT p1.oid, p1.oprcode, p2.oid, p2.oprcode
FROM pg_operator AS p1, pg_operator AS p2
WHERE p1.oprrsortop = p2.oid AND
    (p1.oprname != '=' OR p2.oprname != '<' OR
     p1.oprkind != 'b' OR p2.oprkind != 'b' OR
     p1.oprright != p2.oprleft OR
     p1.oprright != p2.oprright OR
     p1.oprresult != 16 OR
     p2.oprresult != 16 OR
     p1.oprlsortop = 0);

-- A mergejoinable = operator must have a commutator (usually itself)
-- as well as corresponding < and > operators.  Note that the "corresponding"
-- operators have the same L and R input datatypes as the = operator,
-- whereas the operators linked to by oprlsortop and oprrsortop have input
-- datatypes L,L and R,R respectively.

SELECT p1.oid, p1.* FROM pg_operator AS p1
WHERE p1.oprlsortop != 0 AND
      p1.oprcom = 0;

SELECT p1.oid, p1.* FROM pg_operator AS p1
WHERE p1.oprlsortop != 0 AND NOT
      EXISTS(SELECT * FROM pg_operator AS p2 WHERE
	p2.oprname = '<' AND
	p2.oprleft = p1.oprleft AND
	p2.oprright = p1.oprright AND
	p2.oprkind = 'b');

SELECT p1.oid, p1.* FROM pg_operator AS p1
WHERE p1.oprlsortop != 0 AND NOT
      EXISTS(SELECT * FROM pg_operator AS p2 WHERE
	p2.oprname = '>' AND
	p2.oprleft = p1.oprleft AND
	p2.oprright = p1.oprright AND
	p2.oprkind = 'b');
