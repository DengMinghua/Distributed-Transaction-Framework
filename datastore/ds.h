#ifndef DS_H_
#define DS_H_

#include "rpc.h"
#include "mappings.h"

enum DsReqType {
    READ,
    READNLOCK,
    DELETE,
    UNLOCK,
    UPDATE
}

enum DsRespType {
    READ_SUCCESS,
    READ_LOCKED,
    READ_FAILED,

    RNL_SUCCESS,
    RNL_LOCKED,
    RNL_FAILED,

    UNLOCK_SUCCESS,

    UPDATE_SUCCESS
}

struct DsReadReq {
    uint32_t req_type;
    uint32_t address;
    uint32_t length;
}

struct DsWriteReq {
    uint32_t req_type;
    uint32_t address;
    uint32_t length;
}

size_t ds_forge_read_req(RpcReq *rpc_req,
                DsReqType type,
                uint32_t address,
                uint32_t length) {
    DsReadReq * req = rpc_req->req_buf;
    req-> req_type = type;
    req-> address = address;
    req-> length = length;
    return sizeof(DsReadReq);
}

size_t ds_forge_write_req(RpcReq *rpc_req,
                DsReqType type,
                uint32_t des_address,
                uint32_t length,
                uint32_t src_address) {
    DsWriteReq * req = rpc_req->req_buf;
    req-> req_type = type;
    req-> address = address;
    req->length = length;
    uint32_t * val_ptr = (rpc_req->req_buf + sizeof(DsWriteReq));
    memcpy((char*)val_ptr, (char*)src_address, (size_t)length);
    return sizeof(DsWriteReq) + length;
}

#endif
