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

// make query result be printable
ostream &operator<<(ostream &out, const QueryResult &qres) {
    if (qres.column_names != nullptr) {
        for (auto const &column_name: *qres.column_names)
            out << column_name << " ";
        out << endl << "+";
        for (unsigned int i = 0; i < qres.column_names->size(); i++)
            out << "----------+";
        out << endl;
        for (auto const &row: *qres.rows) {
            for (auto const &column_name: *qres.column_names) {
                Value value = row->at(column_name);
                switch (value.data_type) {
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

QueryResult::~QueryResult() {
    // FIXME
}

/**
 * Execute SQL statement based on statement type
 * @param statement SQL statement to execute
 * @return          query execution result
 */
QueryResult *SQLExec::execute(const SQLStatement *statement) {
    // initialize _tables table, if not yet present
    if (SQLExec::tables == nullptr)
        SQLExec::tables = new Tables();

    try {
        switch (statement->type()) {
            case kStmtCreate:
                return create((const CreateStatement *) statement);
            case kStmtDrop:
                return drop((const DropStatement *) statement);
            case kStmtShow:
                return show((const ShowStatement *) statement);
            default:
                return new QueryResult("not implemented");
        }
    } catch (DbRelationError &e) {
        throw SQLExecError(string("DbRelationError: ") + e.what());
    }
}

/**
 * Get the column name and type from a given column definition
 * @param col               given column definition
 * @param column_name       name of column to get
 * @param column_attribute  attribute of column to get
 * @throw                   SQLExecError if data type not recognized
 */ 
void SQLExec::column_definition(const ColumnDefinition *col, Identifier &column_name, ColumnAttribute &column_attribute) {

    column_name = col->name;
    cout << "column_name: " << column_name << endl; // DEL

    switch (col->type) {
        case ColumnDefinition::INT:
            column_attribute.set_data_type(ColumnAttribute::INT);
            cout << "col_type: " << " INT"<< endl; // DEL
            break;
        case ColumnDefinition::TEXT:
            column_attribute.set_data_type(ColumnAttribute::TEXT);
            cout << "col_type: " << " TEXT" << endl; // DEL
            break;
        default:
            throw SQLExecError("data type not implemented");
    }
}

/**
 * Create a table with a given statement
 * @param statement given statement for table creation
 * @return          query execution result
 */ 
QueryResult *SQLExec::create(const CreateStatement *statement) {

    Identifier table_name = statement->tableName;
    cout << "table_name: " << table_name << endl; // DEL

    Identifier column_name;
    ColumnNames column_names;
    ColumnAttribute column_attribute;
    ColumnAttributes column_attributes;

    for (ColumnDefinition *col : *statement->columns) {
        column_definition(col, column_name, column_attribute);
        column_names.push_back(column_name);
        column_attributes.push_back(column_attribute);        
    }

    // add new table info to tables
    ValueDict row; // ValueDict: map<Identifier, Value>
    row["table_name"] = table_name;
    Handle handle; // Handle: pair<BlockID, RecordID> 
    handle = SQLExec::tables->insert(&row); 



    return new QueryResult("created " + table_name);
}

// DROP ...
QueryResult *SQLExec::drop(const DropStatement *statement) {
    return new QueryResult("not implemented"); // FIXME
}

QueryResult *SQLExec::show(const ShowStatement *statement) {
    return new QueryResult("not implemented"); // FIXME
}

QueryResult *SQLExec::show_tables() {
    return new QueryResult("not implemented"); // FIXME
}

QueryResult *SQLExec::show_columns(const ShowStatement *statement) {
    return new QueryResult("not implemented"); // FIXME
}

