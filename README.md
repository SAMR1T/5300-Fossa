# 5300-Fossa
DB Relation Manager project for CPSC5300/4300 at Seattle U, Spring 2020 

**Milestone3:**
Schema Storage - rudimentary implementation of CREATE TABLE, DROP TABLE, SHOW TABLE, SHOW COLUMNS. </br> 
**Milestone 4:**
Indexing Setup - implementation of CREATE INDEX index_name ON table_name [USING {BTREE | HASH}] (col1, col2, ...), SHOW INDEX FROM table_name, DROP INDEX index_name ON table_name. </br>

**General Steps:** </br>
1. Git clone or download this repo
2. Compile the code by runing "make"
3. Run "./sql5300 ../data" (Should make directory of "data" outside the repo first)
4. Use example test commands as below
5. Use "quit" to exit

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
SQL> 
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
SQL> quit
```

If there is failed test or just want to start over the test, delete all data as below:
```sh
$ rm -f data/*
``` 

## Current Status
Milestone 3: All test implementations are successful. </br>
Milestone 4: All test implementations are successful. </br>

## Handoff video
