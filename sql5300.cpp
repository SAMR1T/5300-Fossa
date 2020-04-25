/**
 * sql5300.cpp main file as part of sql5300 DBMS  for milestone 2
 *
 * @author Thomas ficca and Sonali d'souza
 * @see "Seattle University, CPSC4300/5300, Spring 2020"
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "db_cxx.h"
#include "SQLParser.h"
#include "sqlhelper.h"
#include <typeinfo>
#include <sstream>
#include "heap_storage.h"

using namespace std;
using namespace hsql;

/*
 * we allocate and initialize the _DB_ENV global
 */
DbEnv *_DB_ENV;

//forward declare
string unparseStatement(const SQLStatement *statement);
string unparseCreate(const CreateStatement *statement);
string unparseSelect(const SelectStatement *statement);
string getcolumnDefinitionString(const ColumnDefinition *col);
string operatorExpressionToString(const Expr *expr);
string parseTableRef(const TableRef *table);
string expressionToString(const Expr *expr);

/*
 * parses SQL statement exprression
 * @param SQL statement expression
 * @return string form
 */
string expressionToString(const Expr *expr){
  string result;
  switch (expr->type){
      case kExprStar:
        result += "*";
        break;
      case kExprColumnRef:
        if (expr->table != NULL)
          result += string(expr->table) + ".";
      case kExprLiteralString:
        result += expr->name;
        break;
      case kExprLiteralFloat:
        result += to_string(expr->fval);
        break;
      case kExprLiteralInt:
        result += to_string(expr->ival);
        break;
      case kExprFunctionRef:
        result += string(expr->name) + "?" + expr->expr->name;
        break;
      case kExprOperator:
        result += operatorExpressionToString(expr);
        break;
      default:
        result += "???";
        break;    
  }
  if (expr->alias != NULL)
    result += string(" AS ") + expr->alias;
  return result;
}


/*
 * parses the operator expression
 * @param SQL expression that needs to be translated into string 
 * @return string form
 */
string operatorExpressionToString(const Expr *expr){
  if (expr == NULL)
    return "null";

   string result(" ");
    result += expressionToString(expr->expr);
    result += " ";
    switch (expr->opType) 
    {
        case Expr::SIMPLE_OP:
            result += expr->opChar;
            break;
        default:
            result += " undefined ";
    }
    result += " ";
    if (expr->expr2 != NULL) 
     result += expressionToString(expr->expr2);
    return result;
  }

/*
 * parses table reference of SQL select statement
 * @param table reference
 * @return string form 
 */
string parseTableRef(const TableRef *table)
{
    string result("");
    if (table->type != kTableJoin)
    {
        if (table->list == NULL || table->list->size() == 0)
        {
            result += table->name;
            if(table->alias != NULL)
            {
                result += " AS ";
                result += string(table->alias);
            }
        }
        else
        {
            for(uint i =0; i< table->list->size();i++)
            {
                if (i > 0)
                    result += ", ";
                TableRef *tableRef = table->list->at(i);
                result += tableRef->name;
                if(tableRef->alias != NULL)
                {
                    result += " AS ";
                    result += tableRef->getName();
                }
            }
        }
    }
    else
    {
        result += parseTableRef(table->join->left);
        switch(table->join->type)
        {
            case kJoinLeft:
                result += " LEFT JOIN ";
                break;
            case kJoinRight:
                result += " RIGHT JOIN ";
                break;
            case kJoinOuter:
                result += " OUTER JOIN ";
                break;
            case kJoinInner:
                result += " INNER JOIN ";
                break;
            case kJoinLeftOuter:
                result += " LEFT OUTER JOIN ";
                break; 
            case kJoinRightOuter:                                                                                                                                              
                result += " RIGHT OUTER JOIN ";
                break;
            case kJoinCross:
                result += " CROSS JOIN ";
                break;
            case kJoinNatural:
                result += " NATURAL JOIN ";
                break;
        }
        result += parseTableRef(table->join->right);
        if (table->join->condition != NULL)
        {
            result += " ON ";
            result += expressionToString(table->join->condition);
        }
    }
    return result;
}


/*
 * Determines the type of statement
 * create or select statement 
 * @param statement is SQL statement which needs to be translated into string
 * @return translated string
 */
string unparseStatement(const SQLStatement *statement) {
    switch(statement->type()){
        case kStmtCreate :
            return unparseCreate((CreateStatement*) statement);
        case kStmtSelect :
          return unparseSelect((SelectStatement*) statement);
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
 * parse the SQL select statement
 * @param SQL select statement which needs to be translated into string
 * @return string form of SQL select statement
 */
string unparseSelect(const SelectStatement *statement) {
    string result(" SELECT ");
    for(uint i=0; i< statement->selectList->size(); i++)
    {
        if (i > 0)
            result += ", ";
        result += expressionToString(statement->selectList->at(i));
    }
    result += " FROM ";
    result += parseTableRef(statement->fromTable);
    if(statement->whereClause != NULL)
    {
        result += " WHERE ";
        result += expressionToString(statement->whereClause);
    }
    return result;
}


/*
 * parses the SQL column definition into string
 * @param column definition in SQL create statement 
 * @return string containing column name and it's type 
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


/**
 * Main entry point of the sql5300 program
 * @args dbenvpath  the path to the BerkeleyDB database environment
 * displays the translated string
 */
int main (int argc, char *argv[])
{
    // Open/create the db enviroment
    if (argc != 2) {
        cerr << "Usage: cpsc5300: dbenvpath" << endl;
        return 1;
    }
    char *envHome = argv[1];
    cout << "(sql5300: running with database environment at " << envHome << ")" << endl;
    DbEnv env(0U);
    env.set_message_stream(&cout);
    env.set_error_stream(&cerr);
    try {
        env.open(envHome, DB_CREATE | DB_INIT_MPOOL, 0);
    } catch (DbException &exc) {
        cerr << "(sql5300: " << exc.what() << ")";
        exit(1);
    }
    _DB_ENV = &env;

    while(true)
    {
        std::string query = "";
        std::cout<<"SQL> ";
        getline(std::cin, query);
        if (query == "")
            continue;
        if (query == "quit")
            break;
        if (query == "test"){
          cout << "test_heap_storage: " <<
            (test_heap_storage() ? "ok" : "failed") << endl;
          continue;
        }
        try{
            hsql::SQLParserResult* output = hsql::SQLParser::parseSQLString(query);
            if (output->isValid()) {
                for (uint i = 0; i < output->size(); ++i) {
                    std::cout << unparseStatement(output->getStatement(i)) <<std::endl;
                }

                delete output;
            } 
            else {
                std::cout<<"Invalid SQL:"<< query << std::endl;
            }
        }
        catch(...){
            std::cout<<"Parse exception"<<std::endl;

        }
    }
    return EXIT_SUCCESS;
}


