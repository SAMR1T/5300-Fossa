/**
 * heap_storage.cpp - contains implementation of:
 * SlottedPage, HeapFile
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

// Add a new record to the block. Return its id.
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

void SlottedPage::put(RecordID record_id, const Dbt & data) {

}

void SlottedPage::del(RecordID record_id) {}

bool SlottedPage::has_room(u16 size)  {
    u16 free_space = this->end_free - (u16)(4 * (this->num_records + 1));
	return size <= free_space;
}

void SlottedPage::slide(u16 start, u16 end) {
    
}

Dbt* SlottedPage::get(RecordID record_id){
    u16 size, location;
	get_header(size, location, record_id);
	if (location == 0)
		return nullptr; 
	return new Dbt(this->address(location), size);
}

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

// Heap File section



// Allocate a new block for the database file.
// Returns the new empty DbBlock that is managing the records in this block and its block id.
SlottedPage* HeapFile::get_new(void) {
    char block[SlottedPage::BLOCK_SZ];
    memset(block, 0, sizeof(block));
    Dbt data(block, sizeof(block));

    int block_id = ++this->last;
    Dbt key(&block_id, sizeof(block_id));

    // write out an empty block and read it back in so Berkeley DB is managing the memory
    SlottedPage* page = new SlottedPage(data, this->last, true);
    this->db.put(nullptr, &key, &data, 0); // write it out with initialization applied
    this->db.get(nullptr, &key, &data, 0);
    return page;
}

bool test_heap_storage() {return true;}
/* FIXME FIXME FIXME */
