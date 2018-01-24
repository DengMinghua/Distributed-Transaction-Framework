#ifndef DS_H_
#define DS_H_

#define DS_DEBUG 1
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
    uint8_t * local_address;
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

size_t ds_forge_read_resp(Buffer* resp_buf,
                DsRespType type,
                void * local_address,
                void * remote_address,
                size_t length);
#endif
