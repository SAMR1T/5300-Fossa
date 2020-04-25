# 5300-Fossa
DB Relation Manager project for CPSC5300/4300 at Seattle U, Spring 2020

**What does this project currently do?**

Milestone 1
SQL interpreter that currently accepts SQL statements and just returns a string which is the SQL statement string corresponding to the data structure.

Milestone 2 
At this point, the SQL interpreter now has a rudimentry heap storage engine. We have implemented the basic functions needed for HeapTable, but only for two data types: integer and text.

**What statements this project currently support?**

Milestone 1
Select and create statements

Milestone 2
create, create_if_not_exist, open, close, drop, insert( integer, text only), select( ignoring WHERE, GROUP BY and LIMIT), and project

**Example sql statement and output string?**

SQL> create table foo (a text, b integer, c double)

CREATE TABLE foo (a TEXT, b INT, c DOUBLE)

SQL> select * from foo left join goober on foo.x=goober.x

SELECT * FROM foo LEFT JOIN goober ON foo.x = goober.x


**How to run this project?**

1) Run make

2) Run ./sql5300 ../data (Note: Here data is the subdirectory to hold our Berkeley DB database files)

**How to run heap storage test?**

1) Run make

2) Run ./sql5300 ../data (Note: Here data is the subdirectory to hold our Berkeley DB database files)

3) Type test and then hit enter
