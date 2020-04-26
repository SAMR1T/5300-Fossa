# 5300-Fossa
DB Relation Manager project for CPSC5300/4300 at Seattle U, Spring 2020

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
