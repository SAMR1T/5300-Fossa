# 5300-Fossa
DB Relation Manager project for CPSC5300/4300 at Seattle U, Spring 2020

What does this project currently do?

SQL interpreter that currently accepts SQL statements and just returns a string which is the SQL statement string corresponding to the data structure.

What statements this project currently support?

Select and create statements

Example sql statement and output string?

SQL> create table foo (a text, b integer, c double)

CREATE TABLE foo (a TEXT, b INT, c DOUBLE)

SQL> select * from foo left join goober on foo.x=goober.x

SELECT * FROM foo LEFT JOIN goober ON foo.x = goober.x


How to run this project?

1) Run make

2) Run sql5300 ~/sql5300/data (Note: Here data is the subdirectory to hold our Berkeley DB database files)

