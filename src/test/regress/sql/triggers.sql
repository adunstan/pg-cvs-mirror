
create table pkeys (pkey1 int4 not null, pkey2 text not null);
create table fkeys (fkey1 int4, fkey2 text, fkey3 int);
create table fkeys2 (fkey21 int4, fkey22 text, pkey23 int not null);

create index fkeys_i on fkeys (fkey1, fkey2);
create index fkeys2_i on fkeys2 (fkey21, fkey22);
create index fkeys2p_i on fkeys2 (pkey23);

insert into pkeys values (10, '1');
insert into pkeys values (20, '2');
insert into pkeys values (30, '3');
insert into pkeys values (40, '4');
insert into pkeys values (50, '5');
insert into pkeys values (60, '6');
create unique index pkeys_i on pkeys (pkey1, pkey2);

--
-- For fkeys:
-- 	(fkey1, fkey2)	--> pkeys (pkey1, pkey2)
-- 	(fkey3)		--> fkeys2 (pkey23)
--
create trigger check_fkeys_pkey_exist 
	before insert or update on fkeys 
	for each row 
	execute procedure 
	check_primary_key ('fkey1', 'fkey2', 'pkeys', 'pkey1', 'pkey2');

create trigger check_fkeys_pkey2_exist 
	before insert or update on fkeys 
	for each row 
	execute procedure check_primary_key ('fkey3', 'fkeys2', 'pkey23');

--
-- For fkeys2:
-- 	(fkey21, fkey22)	--> pkeys (pkey1, pkey2)
--
create trigger check_fkeys2_pkey_exist 
	before insert or update on fkeys2 
	for each row 
	execute procedure 
	check_primary_key ('fkey21', 'fkey22', 'pkeys', 'pkey1', 'pkey2');

--
-- For pkeys:
-- 	ON DELETE/UPDATE (pkey1, pkey2) CASCADE:
-- 		fkeys (fkey1, fkey2) and fkeys2 (fkey21, fkey22)
--
create trigger check_pkeys_fkey_cascade
	before delete or update on pkeys 
	for each row 
	execute procedure 
	check_foreign_key (2, 'cascade', 'pkey1', 'pkey2', 
	'fkeys', 'fkey1', 'fkey2', 'fkeys2', 'fkey21', 'fkey22');

--
-- For fkeys2:
-- 	ON DELETE/UPDATE (pkey23) RESTRICT:
-- 		fkeys (fkey3)
--
create trigger check_fkeys2_fkey_restrict 
	before delete or update on fkeys2
	for each row 
	execute procedure check_foreign_key (1, 'restrict', 'pkey23', 'fkeys', 'fkey3');

insert into fkeys2 values (10, '1', 1);
insert into fkeys2 values (30, '3', 2);
insert into fkeys2 values (40, '4', 5);
insert into fkeys2 values (50, '5', 3);
-- no key in pkeys
insert into fkeys2 values (70, '5', 3);

insert into fkeys values (10, '1', 2);
insert into fkeys values (30, '3', 3);
insert into fkeys values (40, '4', 2);
insert into fkeys values (50, '5', 2);
-- no key in pkeys
insert into fkeys values (70, '5', 1);
-- no key in fkeys2
insert into fkeys values (60, '6', 4);

delete from pkeys where pkey1 = 30 and pkey2 = '3';
delete from pkeys where pkey1 = 40 and pkey2 = '4';
update pkeys set pkey1 = 7, pkey2 = '70' where pkey1 = 50 and pkey2 = '5';
update pkeys set pkey1 = 7, pkey2 = '70' where pkey1 = 10 and pkey2 = '1';

DROP TABLE pkeys;
DROP TABLE fkeys;
DROP TABLE fkeys2;

create table dup17 (x int4);

create trigger dup17_before 
	before insert on dup17
	for each row 
	execute procedure 
	funny_dup17 ()
;

insert into dup17 values (17);
select count(*) from dup17;
insert into dup17 values (17);
select count(*) from dup17;

drop trigger dup17_before on dup17;

create trigger dup17_after
	after insert on dup17
	for each row 
	execute procedure 
	funny_dup17 ()
;
insert into dup17 values (13);
select count(*) from dup17 where x = 13;
insert into dup17 values (13);
select count(*) from dup17 where x = 13;

DROP TABLE dup17;
