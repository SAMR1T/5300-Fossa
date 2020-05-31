# 5300-Fossa
DB Relation Manager project for CPSC5300/4300 at Seattle U, Spring 2020 

## Sprint Invierno

**Milstone 5:**
Implemented certain INSERT, SELECT, and DELETE statements.

**General Steps for Use:** </br>
1. Git clone or download this repo
2. Compile the code by runing "make"
3. Run "./sql5300 ../data" (Should make directory of "data" outside the repo first)
4. Try example test commands as below
5. Enter "quit" to exit

**Test for M5:**

```sql
SQL> insert into goober (z,y,x) VALUES (9,8,7)
INSERT INTO goober (z, y, x) VALUES (9, 8, 7)
successfully inserted 1 row into goober and 2 indices
SQL> select * from goober
SELECT * FROM goober
x y z 
+----------+----------+----------+
4 5 6 
9 9 9 
7 8 9 
successfully returned 3 rows
```

## Previous Sprints

## Sprint Otoño

**Milestone 3:**
Schema Storage - rudimentary implementation of CREATE TABLE, DROP TABLE, SHOW TABLE, and SHOW COLUMNS. </br> 

**Milestone 4:**
Indexing Setup - rudimentary implementation of CREATE INDEX, SHOW INDEX, and DROP INDEX. </br>

**General Steps:** </br>
1. Git clone or download this repo
2. Compile the code by runing "make"
3. Run "./sql5300 ../data" (Should make directory of "data" outside the repo first)
4. Try example test commands as below
5. Enter "quit" to exit

**Test Steps for M3:** (Create Table, Show Table/Columns and Drop Table)
```sql
SQL> show tables
SHOW TABLES
table_name 
+----------+
successfully returned 0 rows
SQL> show columns from _tables
SHOW COLUMNS FROM _tables
table_name column_name data_type 
+----------+----------+----------+
"_tables" "table_name" "TEXT" 
successfully returned 1 rows
SQL> show columns from _columns
SHOW COLUMNS FROM _columns
table_name column_name data_type 
+----------+----------+----------+
"_columns" "table_name" "TEXT" 
"_columns" "column_name" "TEXT" 
"_columns" "data_type" "TEXT" 
successfully returned 3 rows
SQL> create table foo (id int, data text, x integer, y integer, z integer)
CREATE TABLE foo (id INT, data TEXT, x INT, y INT, z INT)
created foo
SQL> create table foo (goober int)
CREATE TABLE foo (goober INT)
Error: DbRelationError: foo already exists
SQL> create table goo (x int, x text)
Error: DbRelationError: duplicate column goo.x
SQL> show tables
SHOW TABLES
table_name 
+----------+
"foo" 
successfully returned 1 rows
SQL> show columns from foo
SHOW COLUMNS FROM foo
table_name column_name data_type 
+----------+----------+----------+
"foo" "id" "INT" 
"foo" "data" "TEXT" 
"foo" "x" "INT" 
"foo" "y" "INT" 
"foo" "z" "INT" 
successfully returned 5 rows
SQL> drop table foo
DROP TABLE foo
dropped foo
SQL> show tables
SHOW TABLES
table_name 
+----------+
successfully returned 0 rows
SQL> show columns from foo
SHOW COLUMNS FROM foo
table_name column_name data_type 
+----------+----------+----------+
successfully returned 0 rows
```

**Test Steps for M4 (Continued with M3 above):** (Create Index, Show Index and Drop Index)
```sql
SQL> create table goober (x int, y int, z int)
CREATE TABLE goober (x INT, y INT, z INT)
created goober
SQL> show tables
SHOW TABLES
table_name 
+----------+
"goober" 
successfully returned 1 rows
SQL> show columns from goober
SHOW COLUMNS FROM goober
table_name column_name data_type 
+----------+----------+----------+
"goober" "x" "INT" 
"goober" "y" "INT" 
"goober" "z" "INT" 
successfully returned 3 rows
SQL> create index fx on goober (x,y)
CREATE INDEX fx ON goober USING BTREE (x, y)
created index fx
SQL> show index from goober
SHOW INDEX FROM goober
table_name index_name column_name seq_in_index index_type is_unique 
+----------+----------+----------+----------+----------+----------+
"goober" "fx" "x" 1 "BTREE" true 
"goober" "fx" "y" 2 "BTREE" true 
successfully returned 2 rows
SQL> drop index fx from goober
DROP INDEX fx FROM goober
dropped index fx
SQL> show index from goober
SHOW INDEX FROM goober
table_name index_name column_name seq_in_index index_type is_unique 
+----------+----------+----------+----------+----------+----------+
successfully returned 0 rows
SQL> create index fx on goober (x)
CREATE INDEX fx ON goober USING BTREE (x)
created index fx
SQL> show index from goober
SHOW INDEX FROM goober
table_name index_name column_name seq_in_index index_type is_unique 
+----------+----------+----------+----------+----------+----------+
"goober" "fx" "x" 1 "BTREE" true 
successfully returned 1 rows
SQL> create index fx on goober (y,z)
CREATE INDEX fx ON goober USING BTREE (y, z)
Error: DbRelationError: duplicate index goober fx
SQL> show index from goober
SHOW INDEX FROM goober
table_name index_name column_name seq_in_index index_type is_unique 
+----------+----------+----------+----------+----------+----------+
"goober" "fx" "x" 1 "BTREE" true 
successfully returned 1 rows
SQL> create index fyz on goober (y,z)
CREATE INDEX fyz ON goober USING BTREE (y, z)
created index fyz
SQL> show index from goober
SHOW INDEX FROM goober
table_name index_name column_name seq_in_index index_type is_unique 
+----------+----------+----------+----------+----------+----------+
"goober" "fx" "x" 1 "BTREE" true 
"goober" "fyz" "y" 1 "BTREE" true 
"goober" "fyz" "z" 2 "BTREE" true 
successfully returned 3 rows
SQL> drop index fx from goober
DROP INDEX fx FROM goober
dropped index fx
SQL> show index from goober
SHOW INDEX FROM goober
table_name index_name column_name seq_in_index index_type is_unique 
+----------+----------+----------+----------+----------+----------+
"goober" "fyz" "y" 1 "BTREE" true 
"goober" "fyz" "z" 2 "BTREE" true 
successfully returned 2 rows
SQL> drop index fyz from goober
DROP INDEX fyz FROM goober
dropped index fyz
SQL> show index from goober
SHOW INDEX FROM goober
table_name index_name column_name seq_in_index index_type is_unique 
+----------+----------+----------+----------+----------+----------+
successfully returned 0 rows
SQL> drop table goober
DROP TABLE goober
dropped goober
SQL> show tables
SHOW TABLES
table_name
+----------+
successfully returned 0 rows
SQL> quit
```

If there is failed test or just want to start over the test, delete all data as below:
```sh
rm -f ../data/*
``` 

**Current Status**

Milestone 3: All test implementations are successful. </br>
Milestone 4: All test implementations are successful. </br>

**Handoff video**

https://seattleu.instructuremedia.com/embed/75593406-2793-4844-bdbd-dbc5c38e5e5b

## Sprint Verano

**Verano Sprint Handover video link**

https://seattleu.instructuremedia.com/embed/1ac8f7c4-d8a2-4af0-8154-0a0bb1aac86f

**What does this project currently do?**

Milestone 1
SQL interpreter that currently accepts SQL statements and just returns a string which is the SQL statement string corresponding to the data structure.

Milestone 2 
At this point, the SQL interpreter now has a rudimentry heap storage engine. We have implemented the basic functions needed for HeapTable, but only for two data types: integer and text.

**What statements does this project's SQL interpreter currently support?**

Milestone 1
Select and create statements

**What basic functions does this project's HeapTable currently support?**

Milestone 2
create, create_if_not_exist, open, close, drop, insert( integer, text only), select( ignoring WHERE, GROUP BY and LIMIT), and project

**Example sql statement and output string?**

SQL> create table foo (a text, b integer, c double)

CREATE TABLE foo (a TEXT, b INT, c DOUBLE)

SQL> select * from foo left join goober on foo.x=goober.x

SELECT * FROM foo LEFT JOIN goober ON foo.x = goober.x


**How to run this project?**

1) Download and open this project directory

2) Run make

3) Run ./sql5300 ../data (Note: Here data is the subdirectory to hold our Berkeley DB database files. Make sure you create this directory before running this command)

4) To use sql interpreter, start typing sql commands like the examples shown above.

5) To test heap storage, type test and hit enter.
