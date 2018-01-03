#include "ds.h"
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
