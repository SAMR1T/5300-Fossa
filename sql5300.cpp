/*
 *sql5300.cpp main file as part of sql5300 DBMS  for milestone 1
 *authors Sonali Dsouza and Thomas Ficca
 *see Seattle University, CPSC4300/5300, Spring 2020
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "db_cxx.h"
#include "SQLParser.h"
#include "sqlhelper.h"
#include <typeinfo>
#include<sstream>

using namespace std;
using namespace hsql;

string unparseStatement(const SQLStatement *stmt);
string unparseCreate(const CreateStatement *stmt);
string getcolumnDefinitionString(const ColumnDefinition *col);

/*
 * Determines the type of statement
 * create or select statement 
 * @param stmt is SQL statement which needs to be translated into string
 * @return string after translation of SQL statement
 */
string unparseStatement(const SQLStatement *statement) {
    switch(statement->type()){
        case kStmtCreate :
            return unparseCreate((CreateStatement*) statement);
        case kStmtSelect:
            return "";
        default :
            return "";
    }
    return "";
}

/*
 * Parse the create statement
 * @param stmt is the SQL create statement 
 * @return string form of SQL create statement
 */
string unparseCreate(const CreateStatement *statement) {
    string result("CREATE TABLE ");
    result += statement->tableName;
    result += " (";
    result += getcolumnDefinitionString(statement->columns->at(0)); 
    for(uint i=1; i< statement->columns->size(); i++)
    {
        result += ", ";
        result += getcolumnDefinitionString(statement->columns->at(i));
    }
    return result + ")";
}

/*
 * parse the SQL column definition into string
 * @param column definition in SQL create statement 
 * @return string of column name and it's type 
 */
string getcolumnDefinitionString(const ColumnDefinition *col) {
    string result(col->name);
    switch(col->type) {
        case ColumnDefinition::DOUBLE:
            result += " DOUBLE";
            break;
        case ColumnDefinition::INT:
            result += " INT";
            break;
        case ColumnDefinition::TEXT:
            result += " TEXT";
            break;
        default:
            result += " ...";
            break;
    }
    return result;
}


/*
 * main function from where execution starts
 * initialize the db environment
 * initialize the database
 * takes input from user
 * parse the input string into SQL statement
 * pases the SQL statements to unparseStatement method 
 * displays the translated string as a desired output
 */
int main (int argc, char *argv[])
{
    if (argc != 2) {
      cerr << "Usage: ./sql5300 dbenvpath" << endl;
      return EXIT_FAILURE;
    }
    char *envHome = argv[1];
    cout << "(sql5300: Running with database environment at " << envHome << ")" << endl;

    DbEnv env(0U);
    env.set_message_stream(&std::cout);
    env.set_error_stream(&std::cerr);
    env.open(envHome, DB_CREATE | DB_INIT_MPOOL, 0);

    while(true)
    {
        std::string query = "";
        std::cout<<"SQL> ";
        getline(std::cin, query);
        if (query == "")
            continue;
        if (query == "quit")
            break;
        try{
            hsql::SQLParserResult* output = hsql::SQLParser::parseSQLString(query);
            if (output->isValid()) {
                for (uint i = 0; i < output->size(); ++i) {
                    std::cout << unparseStatement(output->getStatement(i)) <<std::endl;
                }

                delete output;
            } 
            else {
                std::cout<<"Statement is not valid"<<std::endl;
            }
        }
        catch(...){
            std::cout<<"Parse exception"<<std::endl;

        }
    }
    return EXIT_SUCCESS;
}


