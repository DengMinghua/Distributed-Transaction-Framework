#include "ds.h"

DataStore::DataStore(const char * file) {
    memory_region = NULL;
    object_region = NULL;
    init_region(file);
}

void DataStore::init_region(const char * file) {
    printf("%s\n", __PRETTY_FUNCTION__);
    void * addr = NULL;
    if (file != "") {
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
    object_region = new(addr) sparse_hash_map();

    return;
}

DsObj* DataStore::lookup(const uint64_t hash_key) {
    return object_region.find(hash_key);
}

bool DataStore::insert(const uint64_t hash_key, const DsObj* obj) {
    object_region[hash_key] = *obj;
    return true;
}

bool DataStore::lock_obj(const uint64_t hash_key) {
    ((object_region.find(hash_key))->obj_hdr).locked = 1;
    return true;
}

bool DataStore::unlock_obj(const uint64_t hash_key) {
    ((object_region.find(hash_key))->obj_hdr).locked = 0;
    return true;
}

bool DataStore::is_obj_locked(const uint64_t hash_key) {
    return ((object_region.find(hash_key))->obj-hdr).locked;
}

size_t file_size(int fd) {
    off_t size = lseek(fd, 0, SEEK_END);
    return size;
}
