--
-- Tables and rules for the view test
--
create table rtest_t1 (a int4, b int4);
create table rtest_t2 (a int4, b int4);
create table rtest_t3 (a int4, b int4);

create view rtest_v1 as select * from rtest_t1;
create rule rtest_v1_ins as on insert to rtest_v1 do instead
	insert into rtest_t1 values (new.a, new.b);
create rule rtest_v1_upd as on update to rtest_v1 do instead
	update rtest_t1 set a = new.a, b = new.b
	where a = current.a;
create rule rtest_v1_del as on delete to rtest_v1 do instead
	delete from rtest_t1 where a = current.a;

--
-- Tables and rules for the constraint update/delete test
--
-- Note:
-- 	psql prevents from putting colons into brackets as
-- 	required for multi action rules. So we define single
-- 	rules for each action required for now
--
create table rtest_system (sysname text, sysdesc text);
create table rtest_interface (sysname text, ifname text);
create table rtest_person (pname text, pdesc text);
create table rtest_admin (pname text, sysname text);

create rule rtest_sys_upd1 as on update to rtest_system do 
	update rtest_interface set sysname = new.sysname 
		where sysname = current.sysname;
create rule rtest_sys_upd2 as on update to rtest_system do 
	update rtest_admin set sysname = new.sysname 
		where sysname = current.sysname;

create rule rtest_sys_del1 as on delete to rtest_system do
	delete from rtest_interface where sysname = current.sysname;
create rule rtest_sys_del2 as on delete to rtest_system do
	delete from rtest_admin where sysname = current.sysname;

create rule rtest_pers_upd as on update to rtest_person do 
	update rtest_admin set pname = new.pname where pname = current.pname;

create rule rtest_pers_del as on delete to rtest_person do 
	delete from rtest_admin where pname = current.pname;

--
-- Tables and rules for the logging test
--
create table rtest_emp (ename char(20), salary money);
create table rtest_emplog (ename char(20), who name, action char(10), newsal money, oldsal money);
create table rtest_empmass (ename char(20), salary money);

create rule rtest_emp_ins as on insert to rtest_emp do
	insert into rtest_emplog values (new.ename, getpgusername(),
			'hired', new.salary, '0.00');

create rule rtest_emp_upd as on update to rtest_emp where new.salary != current.salary do
	insert into rtest_emplog values (new.ename, getpgusername(),
			'honored', new.salary, current.salary);

create rule rtest_emp_del as on delete to rtest_emp do
	insert into rtest_emplog values (current.ename, getpgusername(),
			'fired', '0.00', current.salary);

--
-- Tables and rules for the multiple cascaded qualified instead
-- rule test 
--
create table rtest_t4 (a int4, b text);
create table rtest_t5 (a int4, b text);
create table rtest_t6 (a int4, b text);
create table rtest_t7 (a int4, b text);
create table rtest_t8 (a int4, b text);
create table rtest_t9 (a int4, b text);

create rule rtest_t4_ins1 as on insert to rtest_t4
		where new.a >= 10 and new.a < 20 do instead
	insert into rtest_t5 values (new.a, new.b);

create rule rtest_t4_ins2 as on insert to rtest_t4
		where new.a >= 20 and new.a < 30 do
	insert into rtest_t6 values (new.a, new.b);

create rule rtest_t5_ins as on insert to rtest_t5
		where new.a > 15 do
	insert into rtest_t7 values (new.a, new.b);

create rule rtest_t6_ins as on insert to rtest_t6
		where new.a > 25 do instead
	insert into rtest_t8 values (new.a, new.b);

--
-- Tables and rules for the rule fire order test
--
create table rtest_order1 (a int4);
create table rtest_order2 (a int4, b int4, c text);

create sequence rtest_seq;

create rule rtest_order_r3 as on insert to rtest_order1 do instead
	insert into rtest_order2 values (new.a, nextval('rtest_seq'),
		'rule 3 - this should run 3rd or 4th');

create rule rtest_order_r4 as on insert to rtest_order1
		where a < 100 do instead
	insert into rtest_order2 values (new.a, nextval('rtest_seq'),
		'rule 4 - this should run 2nd');

create rule rtest_order_r2 as on insert to rtest_order1 do
	insert into rtest_order2 values (new.a, nextval('rtest_seq'),
		'rule 2 - this should run 1st');

create rule rtest_order_r1 as on insert to rtest_order1 do instead
	insert into rtest_order2 values (new.a, nextval('rtest_seq'),
		'rule 1 - this should run 3rd or 4th');

--
-- Tables and rules for the instead nothing test
--
create table rtest_nothn1 (a int4, b text);
create table rtest_nothn2 (a int4, b text);
create table rtest_nothn3 (a int4, b text);
create table rtest_nothn4 (a int4, b text);

create rule rtest_nothn_r1 as on insert to rtest_nothn1
	where new.a >= 10 and new.a < 20 do instead (select 1);

create rule rtest_nothn_r2 as on insert to rtest_nothn1
	where new.a >= 30 and new.a < 40 do instead nothing;

create rule rtest_nothn_r3 as on insert to rtest_nothn2
	where new.a >= 100 do instead
	insert into rtest_nothn3 values (new.a, new.b);

create rule rtest_nothn_r4 as on insert to rtest_nothn2
	do instead nothing;

