# 5300-Fossa
DB Relation Manager project for CPSC5300/4300 at Seattle U, Spring 2020

Milestone3: Schema Storage - rudimentary implementation of CREATE TABLE, DROP TABLE, SHOW TABLE, SHOW COLUMNS 

```sql
Test Steps:
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
SQL> quit
```

## Unit Tests
There are some tests for SlottedPage and HeapTable. They can be invoked from the <clode>SQL</code> prompt:
```sql
SQL> test
```
Be aware that failed tests may leave garbage Berkeley DB files lingering in your data directory. 
If you don't care about any data in there, you are advised to just delete them all after a failed test.
```sh
$ rm -f data/*
``` 

## Valgrind (Linux)
To run valgrind (files must be compiled with -ggdb):
```sh
$ valgrind --leak-check=full --suppressions=valgrind.supp ./sql5300 data
```
Note that we've added suppression for the known issues with the Berkeley DB library <em>vis-à-vis</em> valgrind.

