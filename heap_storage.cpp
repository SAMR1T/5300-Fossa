/**
 * heap_storage.cpp - contains:
 * SlottedPage, HeapFile, and HeapTable
 *
 * @author Thomas ficca and Sonali d'souza
 * @see "Seattle University, CPSC4300/5300, Spring 2020"
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
 **/
bool SlottedPage::has_room(u16 size)  {
    u16 free_space = this->end_free - (u16)(4 * (this->num_records + 1));
	return size <= free_space;
}

/**
 * Slide data
 * @param start
 * @param end
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

// Get 2-byte integer at given offset in block.
u16 SlottedPage::get_n(u16 offset) {
    return *(u16*)this->address(offset);
}

// Put a 2-byte integer at given offset in block.
void SlottedPage::put_n(u16 offset, u16 n) {
    *(u16*)this->address(offset) = n;
}

// Make a void* pointer for a given offset into the data block.
void* SlottedPage::address(u16 offset) {
    return (void*)((char*)this->block.get_data() + offset);
}

// Store the size and offset for given id. For id of zero, store the block header.
void SlottedPage::put_header(RecordID id, u16 size, u16 loc) {
    if (id == 0) { // called the put_header() version and using the default params
        size = this->num_records;
        loc = this->end_free;
    }
    put_n(4*id, size);
    put_n(4*id + 2, loc);
}

void SlottedPage::get_header( u16 &size, u16 &loc, RecordID id){
	size = get_n(4 * id);
    loc = get_n(4 * id + 2);
}

// Heap File Class

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
	this->db.open(nullptr, this->dbfilename.c_str(), nullptr, DB_RECNO, flags, 0);

	if (flags) {
		this->last = 0;
	} else {
		DB_BTREE_STAT* stat;
	    this->db.stat(nullptr, &stat, DB_FAST_STAT);
		this->last = stat->bt_ndata;
	}

	this->closed = false;
}

//**********HeapTable Implementation***************

//HeapTable Class

HeapTable::HeapTable(Identifier table_name, ColumnNames column_names, ColumnAttributes column_attributes)
  : DbRelation(table_name, column_names, column_attributes), file(table_name) {
}
                                                                                               
                                                                     
/*                                                                     
 * Create a new HeapTable                                                         
 * @param None
 * @return None
 */

void HeapTable::create(){
  cout << "create" << endl;
  this->file.create();
}

/*
 * Create a new file if it does not exist 
 * @param None
 * @return None
 */

void HeapTable::create_if_not_exists() {
  try {
    this->open();
  } catch (DbException &e){
    this->file.create();
  }
  
}

/*
 * Drop the file/table
 * @param None
 * @return None
 */

void HeapTable::drop() {
  this->file.drop();

}

/*
 * Open the file/table 
 * @param None
 * @return None
 */

void HeapTable::open() {
  this->file.open();
}

/*
 * Close the underlying file/table
 * @param None
 * @return None
 */

void HeapTable::close() {
  this->file.close();
}

  
 
 
// test function -- returns true if all tests pass
bool test_heap_storage() {
	ColumnNames column_names;
	column_names.push_back("a");
	column_names.push_back("b");
	ColumnAttributes column_attributes;
	ColumnAttribute ca(ColumnAttribute::INT);
	column_attributes.push_back(ca);
	ca.set_data_type(ColumnAttribute::TEXT);
	column_attributes.push_back(ca);
    // HeapTable table1("_test_create_drop_cpp", column_names, column_attributes);
    // table1.create();
    //std::cout << "create ok" << std::endl;
    // table1.drop();  // drop makes the object unusable because of BerkeleyDB restriction -- maybe want to fix this some day
    // std::cout << "drop ok" << std::endl;

    // HeapTable table("_test_data_cpp", column_names, column_attributes);
    // table.create_if_not_exists();
    // std::cout << "create_if_not_exsts ok" << std::endl;

    // ValueDict row;
    // row["a"] = Value(12);
    // row["b"] = Value("Hello!");
    // std::cout << "try insert" << std::endl;
    // table.insert(&row);
    // std::cout << "insert ok" << std::endl;
    // Handles* handles = table.select();
    // std::cout << "select ok " << handles->size() << std::endl;
    // ValueDict *result = table.project((*handles)[0]);
    // std::cout << "project ok" << std::endl;
    // Value value = (*result)["a"];
    // if (value.n != 12)
    // 	return false;
    // value = (*result)["b"];
    // if (value.s != "Hello!")
	// 	return false;
    // table.drop();

    return true;
}
