#ifndef DS_REQ_H_
#define DS_REQ_H_

#define DS_DEBUG 1
#include "../mappings/mappings.h"
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
    uint8_t * address;
    size_t length;
};

struct DsReadResp {
    DsRespType resp_type;
    size_t num_blocks;
    size_t length;
};

struct DsWriteReq {
    DsReqType req_type;
    uint8_t * address;
    size_t length;
};

size_t ds_forge_read_req(RpcReq *rpc_req,
                DsReqType type,
                void * address,
                size_t length);

size_t ds_forge_write_req(RpcReq *rpc_req,
                DsReqType type,
                void * des_address,
                size_t length,
                void * src_address);

size_t ds_forge_read_resp(uint8_t* resp_buf,
                DsRespType type,
                void * local_address,
                size_t num_blocks,
                size_t length,
                uint8_t * version);
#endif
