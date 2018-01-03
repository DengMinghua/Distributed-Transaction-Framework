#ifndef DS_H_
#define DS_H_

#include "../rpc/rpc.h"
#include "../mappings/mappings.h"
#include <cstring>

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

    UPDATE_SUCCESS
};

struct DsReadReq {
    uint32_t req_type;
    uint8_t * address;
    uint32_t length;
};

struct DsWriteReq {
    uint32_t req_type;
    uint8_t * address;
    uint32_t length;
};

size_t ds_forge_read_req(RpcReq *rpc_req,
                DsReqType type,
                uint8_t * address,
                uint32_t length) {
    DsReadReq * req = (DsReadReq * )rpc_req->req_buf;
    req-> req_type = type;
    req-> address = address;
    req-> length = length;
    return sizeof(DsReadReq);
}

size_t ds_forge_write_req(RpcReq *rpc_req,
                DsReqType type,
                uint8_t * des_address,
                uint32_t length,
                uint8_t * src_address) {
    DsWriteReq * req = (DsWriteReq * )rpc_req->req_buf;
    req-> req_type = type;
    req-> address = src_address;
    req->length = length;
    uint8_t * val_ptr = (rpc_req->req_buf + sizeof(DsWriteReq));
    memcpy((uint8_t*)val_ptr, (uint8_t*)src_address, (size_t)length);
    return sizeof(DsWriteReq) + length;
}

#endif
