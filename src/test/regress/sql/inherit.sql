--
-- Test inheritance features
--
CREATE TABLE a (aa TEXT);
CREATE TABLE b UNDER a (bb TEXT);
CREATE TABLE c UNDER a (cc TEXT);
CREATE TABLE d UNDER b,c,a (dd TEXT);

INSERT INTO a(aa) VALUES('aaa');
INSERT INTO a(aa) VALUES('aaaa');
INSERT INTO a(aa) VALUES('aaaaa');
INSERT INTO a(aa) VALUES('aaaaaa');
INSERT INTO a(aa) VALUES('aaaaaaa');
INSERT INTO a(aa) VALUES('aaaaaaaa');

INSERT INTO b(aa) VALUES('bbb');
INSERT INTO b(aa) VALUES('bbbb');
INSERT INTO b(aa) VALUES('bbbbb');
INSERT INTO b(aa) VALUES('bbbbbb');
INSERT INTO b(aa) VALUES('bbbbbbb');
INSERT INTO b(aa) VALUES('bbbbbbbb');

INSERT INTO c(aa) VALUES('ccc');
INSERT INTO c(aa) VALUES('cccc');
INSERT INTO c(aa) VALUES('ccccc');
INSERT INTO c(aa) VALUES('cccccc');
INSERT INTO c(aa) VALUES('ccccccc');
INSERT INTO c(aa) VALUES('cccccccc');

INSERT INTO d(aa) VALUES('ddd');
INSERT INTO d(aa) VALUES('dddd');
INSERT INTO d(aa) VALUES('ddddd');
INSERT INTO d(aa) VALUES('dddddd');
INSERT INTO d(aa) VALUES('ddddddd');
INSERT INTO d(aa) VALUES('dddddddd');

SELECT * FROM a;
SELECT * FROM b;
SELECT * FROM c;
SELECT * FROM d;
SELECT * FROM ONLY a;
SELECT * FROM ONLY b;
SELECT * FROM ONLY c;
SELECT * FROM ONLY d;

UPDATE a SET aa='zzzz' WHERE aa='aaaa';
UPDATE ONLY a SET aa='zzzzz' WHERE aa='aaaaa';
UPDATE b SET aa='zzz' WHERE aa='aaa';
UPDATE ONLY b SET aa='zzz' WHERE aa='aaa';
UPDATE a SET aa='zzzzzz' WHERE aa LIKE 'aaa%';

SELECT * FROM a;
SELECT * FROM b;
SELECT * FROM c;
SELECT * FROM d;
SELECT * FROM ONLY a;
SELECT * FROM ONLY b;
SELECT * FROM ONLY c;
SELECT * FROM ONLY d;

UPDATE b SET aa='new';

SELECT * FROM a;
SELECT * FROM b;
SELECT * FROM c;
SELECT * FROM d;
SELECT * FROM ONLY a;
SELECT * FROM ONLY b;
SELECT * FROM ONLY c;
SELECT * FROM ONLY d;

UPDATE a SET aa='new';

DELETE FROM ONLY c WHERE aa='new';

SELECT * FROM a;
SELECT * FROM b;
SELECT * FROM c;
SELECT * FROM d;
SELECT * FROM ONLY a;
SELECT * FROM ONLY b;
SELECT * FROM ONLY c;
SELECT * FROM ONLY d;

DELETE FROM a;

SELECT * FROM a;
SELECT * FROM b;
SELECT * FROM c;
SELECT * FROM d;
SELECT * FROM ONLY a;
SELECT * FROM ONLY b;
SELECT * FROM ONLY c;
SELECT * FROM ONLY d;
