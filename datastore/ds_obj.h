#ifndef DS_OBJ_H_
#define DS_OBJ_H_

#define OBJ_SIZE 4096
#define OBJ_HDR_SIZE 4
#define OBJ_VAL_MAX_SIZE (4096 - OBJ_HDR_SIZE)

struct DsObjHdr {
    uint32_t locked : 1;
    uint32_t version : 19;
    uint32_t value_size : 12;
};

struct DsObj {
    DsObjHdr obj_hdr;
    uint8_t obj[OBJ_MAX_SIZE];
};

typedef uint64_t ObjKey

#endif
