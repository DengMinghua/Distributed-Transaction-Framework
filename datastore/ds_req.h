#ifndef DS_REQ_H_
#define DS_REQ_H_

#define DS_DEBUG 1
#include "../mappings/mappings.h"
#include "../rpc/rpc.h"
#include <cstring>
#include "ds_obj.h"

enum DsReqType {
    DS_READ,
    DS_READNLOCK,
    DS_DELETE,
    DS_UNLOCK,
    DS_UPDATE
};

enum DsRespType {
    READ_SUCCESS,
    READ_LOCKED,
    READ_FAILED,

    RNL_SUCCESS,
    RNL_LOCKED,
    RNL_FAILED,

    UNLOCK_SUCCESS,

    UPDATE_SUCCESS,
    UPDATE_FAILED
};

struct DsReadReq {
    DsReqType req_type;
    uint64_t obj_key;
};

struct DsReadResp {
    DsRespType resp_type;
    DsObj requested_obj;
};

struct DsWriteReq {
    DsReqType req_type;
    uint64_t obj_key;
    DsObj obj;
};

size_t ds_forge_read_req(RpcReq *rpc_req,
                DsReqType type,
                uint64_t obj_key);

size_t ds_forge_write_req(RpcReq *rpc_req,
                DsReqType type,
                uint64_t obj_key,
                DsObj* obj);

size_t ds_forge_read_resp(uint8_t* resp_buf,
                DsRespType type,
                DsObj* requested_obj);
#endif
