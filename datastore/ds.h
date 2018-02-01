#ifndef DS_H_
#define DS_H_

#include "ds_obj.h"
#include "ds_req.h"

#include "../extlib/CRCpp/inc/CRC.h"
#include "../extlib/sparsepp/sparsepp/spp.h"

#include <assert.h>
#include <stdint.h>
#include <cstdio>
#include <sys/types.h>
#include <unistd.h>
#include <cstdlib>
#include <sys/mman.h>
#include <fcntl.h>
#include <cstring>
#include <pthread.h>

#define MAX_REGION_SIZE (4096 * 1024)

class DataStore {
private:
    void * memory_region;
    spp::sparse_hash_map<uint64_t, DsObj> * object_region;
public:
    DataStore(const char * file = "");
    ~DataStore();
    void  init_region(const char * file);
    DsObj* lookup(const uint64_t hash_key);
    bool insert(const uint64_t hash_key, const DsObj* obj);
    bool lock_obj(const uint64_t hash_key);
    bool unlock_obj(const uint64_t hash_key);
    bool is_obj_locked(const uint64_t hash_key);
};

static size_t file_size(int fd);
#endif

