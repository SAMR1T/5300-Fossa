/**
 * heap_storage.cpp - contains:
 * SlottedPage, HeapFile, and HeapTable
 *
 * test function at bottom of file
 *
 * @author Thomas ficca and Sonali d'souza
 * @see "Seattle University, CPSC4300/5300, Spring 2020"
 *
 *
 * for this heap storage engine, the Berkley DB  code is
 * made up of three layers: DbBlock, DbFile, and DbRelation
 * with DbBlock being the lowest.
 *
 * our concrete implementation or interface classes will be:
 * SlottedPage, Heapfile, and HeapTable with SlottedPage
 * being the lowest.
 *
 */

#include "heap_storage.h"
#include <utility>
#include <vector>
#include <cstring>
#include <exception>
#include <map>

using namespace std;

typedef u_int16_t u16;



SlottedPage::SlottedPage(Dbt &block, BlockID block_id, bool is_new) : DbBlock(block, block_id, is_new) {
    if (is_new) {
        this->num_records = 0;
        this->end_free = DbBlock::BLOCK_SZ - 1;
        put_header();
    } else {
        get_header(this->num_records, this->end_free);
    }
}

/**
 * Add a new record to the block. 
 * @param data the Dbt object
 * @return the record ID of the added record
 **/
RecordID SlottedPage::add(const Dbt* data) {
    if (!has_room(data->get_size()))
        throw DbBlockNoRoomError("not enough room for new record");
    u16 id = ++this->num_records;
    u16 size = (u16) data->get_size();
    this->end_free -= size;
    u16 loc = this->end_free + 1;
    put_header();
    put_header(id, size, loc);
    memcpy(this->address(loc), data->get_data(), size);
    return id;
}

/**
 * Replace a existing record with given data 
 * @param data the Dbt object
 * @return None
 **/
void SlottedPage::put(RecordID record_id, const Dbt & data) {
  u16 location, size;
  u16 data_size = (u16)data.get_size();
	get_header(size, location, record_id);
	
	if (data_size < size) {
    memcpy(this->address(location), data.get_data(), data_size);
		slide(location + data_size, location + size);
	}
	else {
		if (!has_room(data_size - size))
			throw DbBlockNoRoomError("No room to replace the record with the given data");
		slide(location, location - (data_size - size));
		memcpy(this->address(location - (data_size - size)), data.get_data(), data_size);
	}

	get_header(size, location, record_id);
	put_header(record_id, data_size, location);
}

/**
 * Delete a record with given record_id
 * @param record_id of the record that needs to be deleted
 * @return None
 **/
void SlottedPage::del(RecordID record_id) {
  u16 location, size;
	get_header(size, location, record_id);
	put_header(record_id, 0, 0);
	slide(location, location + size);
}

/**
 * Check if there is enough room for given size
 * @param size to be checked
 * @return returns a bool
 **/
bool SlottedPage::has_room(u16 size)  {
  u16 free_space = this->end_free - (u16)(4 * (this->num_records + 1));
	return size <= free_space;
}

/**
 * Slide data
 * @param start
 * @param end
 * @return None
 **/
void SlottedPage::slide(u16 start, u16 end) {
	int move = end - start;
	if (move == 0)
		return;

  int bytes = start - (this->end_free + 1U);
  char temp_copy[bytes];
	void* to = this->address((u16)(this->end_free + 1 + move));
	void* from = this->address((u16)(this->end_free + 1));

	memcpy(temp_copy, from, bytes);
	memcpy(to, temp_copy, bytes);

	RecordIDs * rec_ids = ids();
	for (auto const& rec_id : *rec_ids) {
		u16 location, size;
		get_header(size, location, rec_id);

		if (location <= start) {
			location = location + move;
			put_header(rec_id, size, location);
		}
	}
	delete rec_ids;

	this->end_free += move;
	put_header();
}

/**
 * Get record with given record id
 * @param record_id
 * @return a pointer to a new Dbt or a nullptr
 **/
Dbt* SlottedPage::get(RecordID record_id){
  u16 size, location;
	get_header(size, location, record_id);
	if (location == 0)
		return nullptr; 
	return new Dbt(this->address(location), size);
}

/**
 * Get all the record ids
 * @param void
 * @return a pointer
 **/
RecordIDs* SlottedPage::ids(void){
    RecordIDs *recordIds = new RecordIDs();
    for(int i = 1; i < this->num_records + 1; i++){
        if(get(i) != NULL){
            recordIds->push_back(i);
        }
    }
    return recordIds;
}

/*
 * Get 2-byte integer at given offset in block. 
 * @param integer
 * @return an integer
 */
u16 SlottedPage::get_n(u16 offset) {
    return *(u16*)this->address(offset);
}

/*
 * Put a 2-byte integer at given offset in block.
 * @param 2 integers
 * @return None
 */
void SlottedPage::put_n(u16 offset, u16 n) {
    *(u16*)this->address(offset) = n;
}

/*
 * Make a void* pointer for a given offset into the data block.
 * @param intger
 * @return void pointer
 */
 
void* SlottedPage::address(u16 offset) {
    return (void*)((char*)this->block.get_data() + offset);
}

/*
 * Store the size and offset for given id. For id of zero, store
 * the block header.
 * @pararm a real id or zero, a size, and loc
 * @return None
 */ 
void SlottedPage::put_header(RecordID id, u16 size, u16 loc) {
//called the put_header() version and using the default params  
    if (id == 0) { 
        size = this->num_records;
        loc = this->end_free;
    }
    put_n(4*id, size);
    put_n(4*id + 2, loc);
}

/*
 * Get header for a given size, loc, and id
 * @param 2 integers and a recordID
 * @return None
 */
 void SlottedPage::get_header( u16 &size, u16 &loc, RecordID id){
	size = get_n(4 * id);
    loc = get_n(4 * id + 2);
 }


//***************HeapFile Implementation ********************************

// Heap File Class

HeapFile::HeapFile(string name) : DbFile(name), dbfilename(""), last(0), closed(true), db(_DB_ENV, 0) {
	this->dbfilename = this->name + ".db";
}

/**
 * Create a physical file
 * @param none
 * @return None
 **/
void HeapFile::create(void) {
  db_open(DB_CREATE | DB_EXCL);
	SlottedPage * slotted_page = get_new();
	delete slotted_page;
}

/**
 * Delete the physical file.
 * @param none
 * @return None
 **/
void HeapFile::drop(void) {
	close();
	Db db(_DB_ENV, 0);
	db.remove(this->dbfilename.c_str(), nullptr, 0);
}

/**
 * Open a physical file
 * @param none
 * @return None
 **/
void HeapFile::open(void) {
	db_open();
}

/**
 * Close a physical file
 * @param none
 * @return None
 **/
void HeapFile::close(void) {
	this->db.close(0);
	this->closed = true;
}

/**
 * Allocate a new block for the database file.
 * @param none
 * @return Returns the new empty DbBlock that is managing the records in this block and its block id.
 **/
SlottedPage* HeapFile::get_new(void) {
	char block[DbBlock::BLOCK_SZ];
	memset(block, 0, sizeof(block));
	Dbt data(block, sizeof(block));

	int block_id = ++this->last;
	Dbt key(&block_id, sizeof(block_id));

	// write out an empty block and read it back in so Berkeley DB is managing the memory
	SlottedPage* page = new SlottedPage(data, this->last, true);
	this->db.put(nullptr, &key, &data, 0); // write it out with initialization done to it
	delete page;
	this->db.get(nullptr, &key, &data, 0);
	return new SlottedPage(data, this->last);
}

/**
 * Get block from the database file.
 * @param none
 * @return Returns DbBlock and its block id.
 **/
SlottedPage* HeapFile::get(BlockID block_id) {
	Dbt data;
	Dbt key(&block_id, sizeof(block_id));
	this->db.get(nullptr, &key, &data, 0);
	return new SlottedPage(data, block_id, false);
}

/**
 * Write block to the database file.
 * @param none
 * @return None
 **/
void HeapFile::put(DbBlock * block) {
	int block_id = block->get_block_id();
	Dbt key(&block_id, sizeof(block_id));
	this->db.put(nullptr, &key, block->get_block(), 0);
}

/**
 * Get all block ids.
 * @param none
 * @return None
 **/
BlockIDs* HeapFile::block_ids() {
	BlockIDs* block_ids = new BlockIDs();
	for (BlockID block_id = 1; block_id <= this->last; block_id++)
		block_ids->push_back(block_id);
	return block_ids;
}

/**
 * Open berkley db.
 * @param flags
 * @return None
 **/
void HeapFile::db_open(uint flags) {
	if (!this->closed)
		return;

	this->db.set_re_len(DbBlock::BLOCK_SZ); 
	this->db.open(NULL, this->dbfilename.c_str(), NULL, DB_RECNO, flags, 0);

	if (flags) {
		this->last = 0;
	} else {
		  DB_BTREE_STAT* stat;
	    this->db.stat(nullptr, &stat, DB_FAST_STAT);
		  this->last = stat->bt_ndata;
	}

	this->closed = false;

}

//**********HeapTable Implementation****************************

//HeapTable Class
HeapTable::HeapTable(Identifier table_name, ColumnNames column_names, ColumnAttributes column_attributes) :
	DbRelation(table_name, column_names, column_attributes), file(table_name) {
}
                                                                                               
                                                                     
/*                                                                     
 * Create a new HeapTable                                                         
 * @param None
 * @return None
 */
void HeapTable::create(){
  file.create();
}

/*
 * Create a new file if it does not exist 
 * @param None
 * @return None
 */
void HeapTable::create_if_not_exists() {
  try {
    open();
  } catch (DbException &e) {
    file.create();
  }
}

/*
 * Drop the file/table
 * @param None
 * @return None
 */
void HeapTable::drop() {
  file.drop();
}

/*
 * Open the file/table 
 * @param None
 * @return None
 */
void HeapTable::open() {
  file.open();
}

/*
 * Close the underlying file/table
 * @param None
 * @return None
 */
void HeapTable::close() {
  file.close();
}

/*  
 * Insert a row into table
 * @param row to append
 * @retrun handle that returns table with appended row
 */
Handle HeapTable::insert(const ValueDict *row){
  this->open();
  return this->append(this->validate(row));
}

/*
 * Update a row in the table - NEEDS TO BE DONE
 * @param Handle to table to update, value to update with
 * @return None
 */
void HeapTable::update(const Handle handle, const ValueDict *new_values) {
  cout << "Not yet implemented" << endl;
}

/*
 * Delete - NEEDS TO BE DONE
 * @param Handle that returns 
 * @return None
 */
void HeapTable::del(const Handle handle) {
  cout << "Not yet implemented" << endl;
}


/*
 * Loop through all the blocks in the file
 * @param row that will be appended to the table
 * @retrun Handle that has all the block and record
 * id's to every record in the table 
 *
 */
Handles* HeapTable::select() {

  //Vector that holds the Handle vector that holds a pair
  Handles* handles = new Handles();

  BlockIDs* block_ids = file.block_ids();
  for (auto const& block_id: *block_ids){
    SlottedPage* block = file.get(block_id);
    RecordIDs* record_ids = block->ids();
    for (auto const& record_id: *record_ids)
      handles->push_back(Handle(block_id, record_id));
    delete record_ids;
    delete block;
  }
  delete block_ids;
  return handles;
}

Handles* HeapTable::select(const ValueDict *where) {
  Handles* handles = new Handles();
  BlockIDs* block_ids = file.block_ids();
  for (auto const& block_id: *block_ids) {
    SlottedPage* block = file.get(block_id);
    RecordIDs* record_ids = block->ids();
    for (auto const& record_id: *record_ids)
      handles->push_back(Handle(block_id, record_id));
    delete record_ids;
    delete block;
  }
  delete block_ids;
  return handles;
    
}

/*
 * Project ( Insert ) a row into the table
 * @param Handle that holds block id and record id to get every record
 * @return ValueDict for the rows specified by the columns 
 */
ValueDict* HeapTable::project(Handle handle) {
	return project(handle, &this->column_names);
}

/*
 * Return values for the handle
 * @param Handle that holds block id and record id to get every record
 * @return ValueDict for the rows specified by the columns 
 */
ValueDict* HeapTable::project(Handle handle, const ColumnNames * column_names) {
	BlockID block_id = handle.first;
	RecordID record_id = handle.second;
	SlottedPage* block = file.get(block_id);
	Dbt* data = block->get(record_id);
	ValueDict* row = unmarshal(data);
	delete data;
	delete block;

	if (column_names->empty())
		return row;

	ValueDict* values = new ValueDict();
	for (auto const& column_name : * column_names) {
     ValueDict::const_iterator column = row->find(column_name);
		if (column == row->end())
			throw DbRelationError("No column with column name " + column_name);

		(*values)[column_name] = (*row)[column_name];
	}

	delete row;
	return values;
}


/*
 * Verify that all the columns are there and fill in any missing values
 * @param row that will be appended to the table
 * @return ValueDict which has row that will be appnded to the table
 *
 */

ValueDict* HeapTable::validate(const ValueDict *row){

  ValueDict* full_row = new ValueDict();
  for (auto const& column_name: this->column_names){
    ValueDict::const_iterator column = row->find(column_name);//column is the iterator
    Value value;

    if (column ==row->end()) {
      throw DbRelationError("don't know how to handle NULLS, defaults,etc...yet");
    }else{
      //column->second refers to Value in std::map<Identifier, Value> ValueDict
      value = column->second;
    }

    //insert the pair which has an identifier of the column, and the value it
    //refers to the full_row.
    full_row->insert(make_pair(column_name, value));
  }
  return full_row;
}

/*
 * Append a row to the table
 * @param row that will be appended to the table
 * @return  Handle that returns a pair of the last
 * block added and the record id from inserting it
 */

Handle HeapTable::append(const ValueDict *row) {
  Dbt *data = this->marshal(row);
  SlottedPage *block = this->file.get(this->file.get_last_block_id());
  RecordID record_id;
  try {
    record_id = block->add(data); //try to add the row to the slottedPage
  } catch (...) {
    //block was full so create a new one

    //grabs a new block
    block = this->file.get_new();
    //and add the row to the new block and get its record
    record_id = block->add(data);
  }
  this->file.put(block);//put the block in the databases file
  delete data;
  delete block;
  //retrun id of block and record id from inserting into
  //slottedpage as a pair
  return Handle(this->file.get_last_block_id(), record_id);
  
}

/*
 * return the bits to go into the file, caller is responsible
 * for freeing the returned Dbt and its enclosed ret->get_data().
 * @param a row to marshal
 * @return a Dbt structure
 */
 
Dbt* HeapTable::marshal(const ValueDict *row) {
  //more than we need ( we insist that one row fits into
  //DbBlock::BLOCK_SZ
    char *bytes = new char[DbBlock::BLOCK_SZ];
    uint offset = 0;
    uint col_num = 0;
    for (auto const& column_name: this->column_names) {
        ColumnAttribute ca = this->column_attributes[col_num++];
        ValueDict::const_iterator column = row->find(column_name);
        Value value = column->second;
        if (ca.get_data_type() == ColumnAttribute::DataType::INT) {
            *(int32_t*) (bytes + offset) = value.n;
            offset += sizeof(int32_t);
        } else if (ca.get_data_type() == ColumnAttribute::DataType::TEXT) {
            uint size = value.s.length();
            *(u16*) (bytes + offset) = size;
            offset += sizeof(u16);
            //assume ascii for now
            memcpy(bytes+offset, value.s.c_str(), size);  
            offset += size;
        } else {
            throw DbRelationError("Only know how to marshal INT and TEXT");
        }
    }
    char *right_size_bytes = new char[offset];
    memcpy(right_size_bytes, bytes, offset);
    delete[] bytes;
    Dbt *data = new Dbt(right_size_bytes, offset);
    return data;
}

/*
 * reverse the marshaling process done above
 * @param a Dbt structure
 * @return a ValueDict
 */
ValueDict* HeapTable::unmarshal(Dbt * data) {
     
	char* bytes = (char*)data->get_data();
	uint offset = 0;
	uint col_num = 0;

  
  ValueDict* row = new ValueDict();
  Value value;

	for (auto const& column_name : this->column_names) {
		ColumnAttribute ca = this->column_attributes[col_num++];
		value.data_type = ca.get_data_type();

		if (ca.get_data_type() == ColumnAttribute::DataType::INT) {
			value.n = *(int32_t*)(bytes + offset);
			offset += sizeof(int32_t);
      row->insert(make_pair(column_name,value.n));
		}
		else if (ca.get_data_type() == ColumnAttribute::DataType::TEXT) {
			u16 size = *(u16*)(bytes + offset);
			offset += sizeof(u16);

			char text[DbBlock::BLOCK_SZ];
			memcpy(text, bytes + offset, size);
			text[size] = '\0';

			value.s = string(text);  
			offset += size;
      row->insert(make_pair(column_name,value.s));
		}
		else {
			throw DbRelationError("Only know how to unmarshal INT and TEXT");
		}

	}

	return row;
}

/**
 * Print out given failure message and return false.
 * @param message reason for failure
 * @return false
 */
bool assertion_failure(string message) {
    cout << "FAILED TEST: " << message << endl;
    return false;
}

/**
 * Testing function for SlottedPage.
 * @param None
 * @return true if testing succeeded, false otherwise
 *
 */
bool test_slotted_page() {
    // construct one
    char blank_space[DbBlock::BLOCK_SZ];
    Dbt block_dbt(blank_space, sizeof(blank_space));
    SlottedPage slot(block_dbt, 1, true);
    // add a record
    char rec1[] = "hello";
    Dbt rec1_dbt(rec1, sizeof(rec1));
    RecordID id = slot.add(&rec1_dbt);
    if (id != 1)
        return assertion_failure("add id 1");
    // get it back
    Dbt *get_dbt = slot.get(id);
    string expected(rec1, sizeof(rec1));
    string actual((char *) get_dbt->get_data(), get_dbt->get_size());
    if (expected != actual)
        return assertion_failure("get 1 back " + actual);
    delete get_dbt;
    // add another record and fetch it back
    char rec2[] = "goodbye";
    Dbt rec2_dbt(rec2, sizeof(rec2));
    id = slot.add(&rec2_dbt);
    if (id != 2)
        return assertion_failure("add id 2");
    // get it back
    get_dbt = slot.get(id);
    expected = string(rec2, sizeof(rec2));
    actual = string((char *) get_dbt->get_data(), get_dbt->get_size());
    if (expected != actual)
        return assertion_failure("get 2 back " + actual);
    delete get_dbt;
    // test put with expansion (and slide and ids)
    char rec1_rev[] = "something much bigger";
    rec1_dbt = Dbt(rec1_rev, sizeof(rec1_rev));
    slot.put(1, rec1_dbt);
    // check both rec2 and rec1 after expanding put
    get_dbt = slot.get(2);
    expected = string(rec2, sizeof(rec2));
    actual = string((char *) get_dbt->get_data(), get_dbt->get_size());
    if (expected != actual)
        return assertion_failure("get 2 back after expanding put of 1 " + actual);
    delete get_dbt;
    get_dbt = slot.get(1);
    expected = string(rec1_rev, sizeof(rec1_rev));
    actual = string((char *) get_dbt->get_data(), get_dbt->get_size());
    if (expected != actual)
        return assertion_failure("get 1 back after expanding put of 1 " + actual);
    delete get_dbt;
    // test put with contraction (and slide and ids)
    rec1_dbt = Dbt(rec1, sizeof(rec1));
    slot.put(1, rec1_dbt);
    // check both rec2 and rec1 after contracting put
    get_dbt = slot.get(2);
    expected = string(rec2, sizeof(rec2));
    actual = string((char *) get_dbt->get_data(), get_dbt->get_size());
    if (expected != actual)
        return assertion_failure("get 2 back after contracting put of 1 " + actual);
    delete get_dbt;
    get_dbt = slot.get(1);
    expected = string(rec1, sizeof(rec1));
    actual = string((char *) get_dbt->get_data(), get_dbt->get_size());
    if (expected != actual)
        return assertion_failure("get 1 back after contracting put of 1 " + actual);
    delete get_dbt;
    // test del (and ids)
    RecordIDs *id_list = slot.ids();
    if (id_list->size() != 2 || id_list->at(0) != 1 || id_list->at(1) != 2)
        return assertion_failure("ids() with 2 records");
    delete id_list;
    slot.del(1);
    id_list = slot.ids();
    if (id_list->size() != 1 || id_list->at(0) != 2)
        return assertion_failure("ids() with 1 record remaining");
    delete id_list;
    get_dbt = slot.get(1);
    if (get_dbt != nullptr)
        return assertion_failure("get of deleted record was not null");
    // try adding something too big
    rec2_dbt = Dbt(nullptr, DbBlock::BLOCK_SZ - 10); // too big, but only because we have a record in there
    try {
        slot.add(&rec2_dbt);
        return assertion_failure("failed to throw when add too big");
    } catch (const DbBlockNoRoomError &exc) {
        // test succeeded - this is the expected path
    } catch (...) {
        // Note that this won't catch segfault signals -- but in that case we also know the test failed
        return assertion_failure("wrong type thrown when add too big");
    }
    return true;
}


/*
 * test heap_storage function
 * @param simply enter "test" on the SQl prompt
 * @return function returns true if all test pass
 */
bool test_heap_storage() {

  //test SlottedPage
  test_slotted_page();
    

	ColumnNames column_names;
	column_names.push_back("a");
	column_names.push_back("b");
	ColumnAttributes column_attributes;
	ColumnAttribute ca(ColumnAttribute::INT);
	column_attributes.push_back(ca);
	ca.set_data_type(ColumnAttribute::TEXT);
	column_attributes.push_back(ca);
  HeapTable table1("_test_create_drop_cpp", column_names, column_attributes);
  table1.create();
  std::cout << "create ok" << std::endl;
  table1.drop();  // drop makes the object unusable because of BerkeleyDB restriction -- maybe want to fix this some day
  std::cout << "drop ok" << std::endl;

  HeapTable table("_test_data_cpp", column_names, column_attributes);
  table.create_if_not_exists();
  std::cout << "create_if_not_exsts ok" << std::endl;

  ValueDict row;
  row["a"] = Value(12);
  row["b"] = Value("Hello!");
  std::cout << "try insert" << std::endl;
  table.insert(&row);
  std::cout << "insert ok" << std::endl;
  Handles* handles = table.select();
  std::cout << "select ok " << handles->size() << std::endl;
  ValueDict *result = table.project((*handles)[0]);
  std::cout << "project ok" << std::endl;
  Value value = (*result)["a"];
  if (value.n != 12)
    return false;
  value = (*result)["b"];
  if (value.s != "Hello!")
  return false;
  table.drop();

  return true;
}
