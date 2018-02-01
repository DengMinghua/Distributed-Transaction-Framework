#include "../rpc/rpc.h"
#include "ds_req.h"
size_t ds_forge_read_req(RpcReq *rpc_req,
                DsReqType type,
                uint64_t obj_key) {
    DsReadReq * req = (DsReadReq * )rpc_req->req_buf;
    // req-> req_type = type;
    req-> obj_key = obj_key;

#ifdef DS_DEBUG
    printf("%s\n", __PRETTY_FUNCTION__);
    printf("\tRead request forged:\n");
    printf("\trequest type:\t%d\n\t-read object with key %ld\n",
                    (long) req->obj_key);
#endif

    return sizeof(DsReadReq);
}

size_t ds_forge_write_req(RpcReq *rpc_req,
                DsReqType type,
                uint64_t obj_key,
                DsObj* obj) {
    DsWriteReq * req = (DsWriteReq * )rpc_req->req_buf;
    // req-> req_type = type;
    req-> obj_key = obj_key;
    req-> obj = *obj;

    return sizeof(DsWriteReq);
}

size_t ds_forge_read_resp(uint8_t* resp_buf,
                DsRespType type,
                DsObj* requested_obj) {
    DsReadResp * read_resp = (DsReadResp*)resp_buf;

    //read_resp->resp_type = type;
    read_resp->requested_obj = *requested_obj;
    resp_buf += sizeof(DsReadResp);

    return sizeof(DsReadResp);
}
