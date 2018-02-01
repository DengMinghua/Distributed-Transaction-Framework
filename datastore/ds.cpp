#include "ds.h"

DataStore::DataStore(const char * file) {
    memory_region = NULL;
    object_region = NULL;
    init_region("");
}

DataStore::~DataStore() {
	printf("done\n");
	if (object_region != NULL) 
	    delete object_region;
	if (memory_region != NULL)
	    free(memory_region);
	printf("done\n");
}

void DataStore::init_region(const char * file) {
    printf("%s\n", __PRETTY_FUNCTION__);
    void * addr = NULL;
    if (file != "") {
	printf("see here!\n");
	int fd = open(file, O_RDWR | O_NOATIME | O_CREAT, S_IRWXU);
	int f_size = file_size(fd);
	assert(MAX_REGION_SIZE < f_size);
	addr = (void *)mmap(NULL, MAX_REGION_SIZE, PROT_READ | PROT_WRITE,
		MAP_SHARED, fd, 0);
    }
    else {
	addr = malloc(MAX_REGION_SIZE);
    }
    memory_region = addr;
    object_region = new(addr) spp::sparse_hash_map<uint64_t, DsObj>;

    return;
}

DsObj* DataStore::lookup(const uint64_t hash_key) {
    spp::sparse_hash_map<uint64_t, DsObj>::iterator 
	it = (*object_region).find(hash_key);
    if (it != (*object_region).end())
	return &(it->second);
    else 
	return NULL;
}


bool DataStore::insert(const uint64_t hash_key, const DsObj* obj) {
    (*object_region)[hash_key] = *obj;
    return true;
}

bool DataStore::lock_obj(const uint64_t hash_key) {
    if (lookup(hash_key) != NULL) {
	((((*object_region).find(hash_key))->second).obj_hdr).locked = 1;
	return true;
    }
    else
	return false;
}

bool DataStore::unlock_obj(const uint64_t hash_key) {
    if (lookup(hash_key) != NULL) {
	((((*object_region).find(hash_key))->second).obj_hdr).locked = 0;
	return true;
    }
    else
	return false;
}

bool DataStore::is_obj_locked(const uint64_t hash_key) {
    if (lookup(hash_key) != NULL) 
    return ((((*object_region).find(hash_key))->second).obj_hdr).locked; 
    else return false;
}

size_t file_size(int fd) {
    off_t size = lseek(fd, 0, SEEK_END);
    return size;
}
