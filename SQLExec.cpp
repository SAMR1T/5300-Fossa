/**
 * @file SQLExec.cpp - implementation of SQLExec class
 * @author Kevin Lundeen
 * @see "Seattle University, CPSC5300, Spring 2020"
 */
#include "SQLExec.h"

using namespace std;
using namespace hsql;

// define static data
Tables *SQLExec::tables = nullptr;
Indices *SQLExec::indices = nullptr;

// make query result be printable
ostream &operator<<(ostream &out, const QueryResult &qres)
{
    if (qres.column_names != nullptr)
    {
        for (auto const &column_name : *qres.column_names)
            out << column_name << " ";
        out << endl
            << "+";
        for (unsigned int i = 0; i < qres.column_names->size(); i++)
            out << "----------+";
        out << endl;
        for (auto const &row : *qres.rows)
        {
            for (auto const &column_name : *qres.column_names)
            {
                Value value = row->at(column_name);
                switch (value.data_type)
                {
                case ColumnAttribute::INT:
                    out << value.n;
                    break;
                case ColumnAttribute::TEXT:
                    out << "\"" << value.s << "\"";
                    break;
                default:
                    out << "???";
                }
                out << " ";
            }
            out << endl;
        }
    }
    out << qres.message;
    return out;
}

/**
 * Destructor
 */
QueryResult::~QueryResult()
{
    if (column_names != nullptr)
        delete column_names;
    if (column_attributes != nullptr)
        delete column_attributes;
    if (rows != nullptr)
    {
        for (auto row : *rows)
            delete row;
        delete rows;
    }
}

/**
 * Execute SQL statement based on statement type
 * @param statement SQL statement to execute
 * @return          query execution result
 */
QueryResult *SQLExec::execute(const SQLStatement *statement)
{
    // initialize _tables table, if not yet present
    if (SQLExec::tables == nullptr)
        SQLExec::tables = new Tables();

    // initialize _indices table, if not yet present
    if (SQLExec::indices == nullptr)
    {
        SQLExec::indices = new Indices();
    }

    try
    {
        switch (statement->type())
        {
        case kStmtCreate:
            return create((const CreateStatement *)statement);
        case kStmtDrop:
            return drop((const DropStatement *)statement);
        case kStmtShow:
            return show((const ShowStatement *)statement);
        default:
            return new QueryResult("not implemented");
        }
    }
    catch (DbRelationError &e)
    {
        throw SQLExecError(string("DbRelationError: ") + e.what());
    }
}

/**
 * Pull out column name and attributes from AST's column definition clause
 * @param col                AST column definition
 * @param column_name        returned by reference
 * @param column_attributes  returned by reference
 */
void SQLExec::column_definition(const ColumnDefinition *col, Identifier &column_name, ColumnAttribute &column_attribute)
{
    column_name = col->name;

    switch (col->type)
    {
    case ColumnDefinition::INT:
        column_attribute.set_data_type(ColumnAttribute::INT);
        break;
    case ColumnDefinition::TEXT:
        column_attribute.set_data_type(ColumnAttribute::TEXT);
        break;
    default:
        throw SQLExecError("data type not implemented");
    }
}

/**
 * create a table or an index based on given statement
 * @param statement given statement for creation
 * @return          query execution result
 */
QueryResult *SQLExec::create(const CreateStatement *statement)
{
    switch (statement->type)
    {
    case CreateStatement::kTable:
        return create_table(statement);
    case CreateStatement::kIndex:
        return create_index(statement);
    default:
        return new QueryResult("Only CREATE TABLE and CREATE INDEX are implemented");
    }
}

/**
 * Create a table with a given statement
 * @param statement given statement for table creation
 * @return          query execution result
 */
QueryResult *SQLExec::create_table(const CreateStatement *statement)
{
    Identifier table_name = statement->tableName;
    Identifier column_name;
    ColumnNames column_names;
    ColumnAttribute column_attribute;
    ColumnAttributes column_attributes;

    for (ColumnDefinition *col : *statement->columns)
    {
        column_definition(col, column_name, column_attribute);
        column_names.push_back(column_name);
        column_attributes.push_back(column_attribute);
    }

    // update _tables and _columns
    ValueDict row; // ValueDict: map<Identifier, Value>
    // handle holder for exception
    Handle table_handle; // Handle: pair<BlockID, RecordID>
    Handle column_handle;
    Handles column_handles;

    try
    {
        // add new table info to _tables
        row["table_name"] = table_name;
        table_handle = SQLExec::tables->insert(&row);

        // add new columns info to _columns
        DbRelation &columns = SQLExec::tables->get_table(Columns::TABLE_NAME);
        try
        {
            for (uint i = 0; i < column_names.size(); ++i)
            {
                row["column_name"] = column_names[i];

                switch (column_attributes[i].get_data_type())
                {
                case ColumnAttribute::INT:
                    row["data_type"] = Value("INT");
                    break;
                case ColumnAttribute::TEXT:
                    row["data_type"] = Value("TEXT");
                    break;
                default:
                    throw SQLExecError("data type not implemented");
                }
                column_handle = columns.insert(&row);
                column_handles.push_back(column_handle);
            }

            // create table file
            DbRelation &table = SQLExec::tables->get_table(table_name);
            if (statement->ifNotExists)
                table.create_if_not_exists();
            else
                table.create();
        }
        catch (exception &e)
        {
            try
            {
                // remove added columns
                for (auto const handle : column_handles)
                    columns.del(handle);
            }
            catch (...)
            {
            }
            throw;
        }
    }
    catch (exception &e)
    {
        try
        {
            // remove added table
            SQLExec::tables->del(table_handle);
        }
        catch (...)
        {
        }
        throw;
    }
    return new QueryResult("created " + table_name);
}

/**
 * Create index with a given statement
 * @param statement given statement for index creation
 * @return          query execution result
 */
QueryResult *SQLExec::create_index(const CreateStatement *statement)
{
    Identifier table_name = statement->tableName;
    Identifier index_name = statement->indexName;

    ValueDict row;

    row["table_name"] = Value(table_name);
    row["index_name"] = Value(index_name);

    Identifier index_type = statement->indexType;
    row["index_type"] = Value(index_type);
    if (index_type == "BTREE")
    {
        row["is_unique"] = true;
    }
    else
    {
        row["is_unique"] = false;
    }

    //row["is_unique"] = (index_type == "BTREE" ? true : false);

    // for rollback in exception
    Handle index_handle;
    Handles index_handles;
    int seq_in_index = 1;

    try
    {
        for (auto const &col : *statement->indexColumns)
        {
            row["column_name"] = Value(col);
            row["seq_in_index"] = Value(seq_in_index++);

            index_handle = SQLExec::indices->insert(&row);
            index_handles.push_back(index_handle);
        }
        // create index
        DbIndex &index = SQLExec::indices->get_index(table_name, index_name);
        index.create();
    }
    catch (exception &e)
    {
        try
        {
            // rollback
            for (auto const handle : index_handles)
                SQLExec::indices->del(handle);
        }
        catch (...)
        {
        }
        throw;
    }
    return new QueryResult("created index " + index_name);
}

/**
 * Drop a table or an index based on given statement
 * @param statement given statement for removal
 * @return          query execution result
 */
QueryResult *SQLExec::drop(const DropStatement *statement)
{
    switch (statement->type)
    {
    case DropStatement::kTable:
        return drop_table(statement);
    case DropStatement::kIndex:
        return drop_index(statement);
    default:
        return new QueryResult("Only DROP TABLE and CREATE INDEX are implemented");
    }
}

/**
 * Drop a table with a given statement
 * @param statement given statement for table removal
 * @return          query execution result
 */
QueryResult *SQLExec::drop_table(const DropStatement *statement)
{
    Identifier table_name = statement->name;

    if (table_name == Tables::TABLE_NAME || table_name == Columns::TABLE_NAME)
        throw SQLExecError("cannot drop a schema table");

    ValueDict row;
    row["table_name"] = Value(table_name);

    DbRelation &table = SQLExec::tables->get_table(table_name);
    DbRelation &columns = SQLExec::tables->get_table(Columns::TABLE_NAME);
    Handles *column_handles = columns.select(&row);
    Handles *table_handle = SQLExec::tables->select(&row);

    // Remove indices before dropping the table
    Handles *index_handles = SQLExec::indices->select(&row);

    // Remove indices from db
    for (auto const &handle : *index_handles)
    {
        ValueDict *index_attributes = SQLExec::indices->project(handle);
        DbIndex &index = SQLExec::indices->get_index(table_name, index_attributes->at("index_name").s);
        index.drop();
    }
    // Remove indices from schema table
    for (auto const &handle : *index_handles)
    {
        SQLExec::indices->del(handle);
    }
    delete index_handles;

    try
    {
        // remove columns from _columns
        for (auto const &handle : *column_handles)
            columns.del(handle);

        // remove table file
        table.drop();

        // remove table from _table
        SQLExec::tables->del(*table_handle->begin());
    }
    catch (...)
    {
        delete table_handle;
        delete column_handles;

        throw SQLExecError("table deletion failed");
    }
    delete table_handle;
    delete column_handles;

    return new QueryResult("dropped " + table_name);
}

/**
 * Drop an index with a given statement
 * @param statement given statement for index removal
 * @return          query execution result
 */
QueryResult *SQLExec::drop_index(const DropStatement *statement)
{
    // Get table_name and index_name
    Identifier table_name = statement->name;
    Identifier index_name = statement->indexName;

    // Get index from the database DbIndex
    DbIndex &index = SQLExec::indices->get_index(table_name, index_name);
    ValueDict where;
    where["table_name"] = Value(table_name);
    where["index_name"] = Value(index_name);
    Handles *index_handles = SQLExec::indices->select(&where);
    index.drop();
    // Remove every index through index handles
    for (auto const &handle : *index_handles)
    {
        SQLExec::indices->del(handle);
    }
    delete index_handles;

    return new QueryResult("dropped index " + index_name);
}

/**
 * SHOW Query type based on statement by user call
 * @param statement given statement for show option
 * @return          query execution result
 */
QueryResult *SQLExec::show(const ShowStatement *statement)
{
    switch (statement->type)
    {
    case ShowStatement::kTables:
        return show_tables();
    case ShowStatement::kColumns:
        return show_columns(statement);
    case ShowStatement::kIndex:
        return show_index(statement);
    default:
        throw SQLExecError("Invalid SHOW type. Only show tables or columns.");
    }
}

/**
 * SHOW index
 * @return query execution result
 */
QueryResult *SQLExec::show_index(const ShowStatement *statement)
{
    // Get table name and prepare column header names
    Identifier table_name = statement->tableName;
    ColumnNames *column_names = new ColumnNames;
    column_names->push_back("table_name");   // e.g. goober
    column_names->push_back("index_name");   // e.g. fx
    column_names->push_back("column_name");  // e.g. x/y/z
    column_names->push_back("seq_in_index"); // e.g. 1
    column_names->push_back("index_type");   // e.g. BTREE / HASH
    column_names->push_back("is_unique");    // e.g. Ture if BTREE, False if HASH

    // Get indices from specific table
    ValueDict where;
    where["table_name"] = Value(statement->tableName);
    Handles *index_handles = SQLExec::indices->select(&where);
    u_long number_of_rows = index_handles->size(); // Number of rows returned in result

    // Get all column content and name of the indices table
    ValueDicts *rows = new ValueDicts;
    for (auto const &handle : *index_handles)
    {
        ValueDict *row = SQLExec::indices->project(handle, column_names);
        rows->push_back(row);
    }
    delete index_handles;

    return new QueryResult(column_names, nullptr, rows,
                           " successfully returned " + to_string(number_of_rows) + " rows");
}

/**
 * SHOW tables of a schema
 * @return query execution result
 */
QueryResult *SQLExec::show_tables()
{
    //Get columns for a specific table
    ColumnNames *column_names = new ColumnNames;
    column_names->push_back("table_name");

    ColumnAttributes *column_attributes = new ColumnAttributes;
    column_attributes->push_back(ColumnAttribute(ColumnAttribute::TEXT));

    Handles *handles = SQLExec::tables->select();
    // Minus the "_tables", "_columns" and "_indices". Number of returned rows in result.
    u_long number_of_rows = handles->size() - 3;

    ValueDicts *rows = new ValueDicts;
    for (auto const &handle : *handles)
    {
        ValueDict *row = SQLExec::tables->project(handle, column_names);
        // Add tables only if not "_tables" or "_columns" or "_indices"
        if (row->at("table_name").s != Tables::TABLE_NAME && row->at("table_name").s != Columns::TABLE_NAME &&
            row->at("table_name").s != Indices::TABLE_NAME)
            rows->push_back(row);
        else
            delete row;
    }
    delete handles;

    return new QueryResult(column_names, column_attributes, rows,
                           "successfully returned " + to_string(number_of_rows) + " rows");
}

/**
 * SHOW columns of a table
 * @param statement given statement for selected table
 * @return          query execution result
 */
QueryResult *SQLExec::show_columns(const ShowStatement *statement)
{
    //Get tables
    DbRelation &columns = SQLExec::tables->get_table(Columns::TABLE_NAME);

    //Add column names with the table name, column name, and data type
    ColumnNames *column_names = new ColumnNames;
    column_names->push_back("table_name");
    column_names->push_back("column_name");
    column_names->push_back("data_type");

    ColumnAttributes *column_attributes = new ColumnAttributes;
    column_attributes->push_back(ColumnAttribute(ColumnAttribute::TEXT));

    //Get columns from specific table in statement
    ValueDict where;
    where["table_name"] = Value(statement->tableName);
    Handles *handles = columns.select(&where);
    u_long number_of_rows = handles->size(); // Number of returned rows in result

    ValueDicts *rows = new ValueDicts;
    for (auto const &handle : *handles)
    {
        ValueDict *row = columns.project(handle, column_names);
        rows->push_back(row);
    }
    delete handles;
    return new QueryResult(column_names, column_attributes, rows,
                           "successfully returned " + to_string(number_of_rows) + " rows");
}
