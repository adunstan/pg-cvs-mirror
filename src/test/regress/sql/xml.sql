CREATE TABLE xmltest (
    id int,
    data xml
);

INSERT INTO xmltest VALUES (1, '<value>one</value>');
INSERT INTO xmltest VALUES (2, '<value>two</value>');
INSERT INTO xmltest VALUES (3, '<wrong');

SELECT * FROM xmltest;


SELECT xmlcomment('test');
SELECT xmlcomment('-test');
SELECT xmlcomment('test-');
SELECT xmlcomment('--test');
SELECT xmlcomment('te st');


SELECT xmlconcat(xmlcomment('hello'),
                 xmlelement(NAME qux, 'foo'),
                 xmlcomment('world'));

SELECT xmlconcat('hello', 'you');
SELECT xmlconcat(1, 2);
SELECT xmlconcat('bad', '<syntax');
SELECT xmlconcat('<foo/>', NULL, '<?xml version="1.1" standalone="no"?><bar/>');
SELECT xmlconcat('<?xml version="1.1"?><foo/>', NULL, '<?xml version="1.1" standalone="no"?><bar/>');


SELECT xmlelement(name element,
                  xmlattributes (1 as one, 'deuce' as two),
                  'content');

SELECT xmlelement(name element,
                  xmlattributes ('unnamed and wrong'));

SELECT xmlelement(name element, xmlelement(name nested, 'stuff'));

SELECT xmlelement(name employee, xmlforest(name, age, salary as pay)) FROM emp;

SELECT xmlelement(name duplicate, xmlattributes(1 as a, 2 as b, 3 as a));

SELECT xmlelement(name num, 37);
SELECT xmlelement(name foo, text 'bar');
SELECT xmlelement(name foo, xml 'bar');
SELECT xmlelement(name foo, text 'b<a/>r');
SELECT xmlelement(name foo, xml 'b<a/>r');
SELECT xmlelement(name foo, array[1, 2, 3]);
SET xmlbinary TO base64;
SELECT xmlelement(name foo, bytea 'bar');
SET xmlbinary TO hex;
SELECT xmlelement(name foo, bytea 'bar');


SELECT xmlparse(content 'abc');
SELECT xmlparse(content '<abc>x</abc>');

SELECT xmlparse(document 'abc');
SELECT xmlparse(document '<abc>x</abc>');


SELECT xmlpi(name foo);
SELECT xmlpi(name xmlstuff);
SELECT xmlpi(name foo, 'bar');
SELECT xmlpi(name foo, 'in?>valid');
SELECT xmlpi(name foo, null);
SELECT xmlpi(name xmlstuff, null);
SELECT xmlpi(name foo, '   bar');

SELECT xmlroot(xml '<foo/>', version no value, standalone no value);
SELECT xmlroot(xml '<foo/>', version '2.0');
SELECT xmlroot(xml '<foo/>', version no value, standalone yes);
SELECT xmlroot(xml '<?xml version="1.1"?><foo/>', version no value, standalone yes);
SELECT xmlroot(xmlroot(xml '<foo/>', version '1.0'), version '1.1', standalone no);
SELECT xmlroot('<?xml version="1.1" standalone="yes"?><foo/>', version no value, standalone no);
SELECT xmlroot('<?xml version="1.1" standalone="yes"?><foo/>', version no value, standalone no value);
SELECT xmlroot('<?xml version="1.1" standalone="yes"?><foo/>', version no value);


SELECT xmlroot (
  xmlelement (
    name gazonk,
    xmlattributes (
      'val' AS name,
      1 + 1 AS num
    ),
    xmlelement (
      NAME qux,
      'foo'
    )
  ),
  version '1.0',
  standalone yes
);


SELECT xmlserialize(content data as character varying) FROM xmltest;


SELECT xml '<foo>bar</foo>' IS DOCUMENT;
SELECT xml '<foo>bar</foo><bar>foo</bar>' IS DOCUMENT;
SELECT xml '<abc/>' IS NOT DOCUMENT;
SELECT xml 'abc' IS NOT DOCUMENT;
SELECT '<>' IS NOT DOCUMENT;


SELECT xmlagg(data) FROM xmltest;
SELECT xmlagg(data) FROM xmltest WHERE id > 10;
SELECT xmlelement(name employees, xmlagg(xmlelement(name name, name))) FROM emp;


-- Check mapping SQL identifier to XML name

SELECT xmlpi(name ":::_xml_abc135.%-&_");
SELECT xmlpi(name "123");


PREPARE foo (xml) AS SELECT xmlconcat('<foo/>', $1);

SET XML OPTION DOCUMENT;
EXECUTE foo ('<bar/>');
EXECUTE foo ('bad');

SET XML OPTION CONTENT;
EXECUTE foo ('<bar/>');
EXECUTE foo ('good');
