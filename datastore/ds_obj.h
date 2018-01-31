#ifndef DS_OBJ_H_
#define DS_OBJ_H_

#define OBJ_MAX_SIZE 4096
#define OBJ_HDR_SIZE 4
#define OBJ_VAL_MAX_SIZE (4096 - OBJ_HDR_SIZE)

#include <cstring>
#include <cstdint>
struct DsObjHdr {
    uint32_t locked : 1;
    uint32_t version : 19;
    uint32_t value_size : 12;
};

struct DsObj {
    DsObjHdr obj_hdr;
    uint8_t obj[OBJ_MAX_SIZE];

    DsObj(uint8_t* val, size_t val_size, bool locked = false) {
        memcpy(obj, val, val_size);
        obj_hdr.value_size = val_size;
        obj_hdr.locked = locked;
        obj_hdr.version = 0;
    }

    DsObj() {
        obj_hdr.locked = false;
        obj_hdr.version = 0;
    }

    inline void update(uint8_t* val, size_t val_size) {
        memcpy(obj, val, val_size);
        obj_hdr.value_size = val_size;
    }
};

typedef uint64_t ObjKey;

#endif
