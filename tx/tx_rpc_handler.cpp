#include "tx.h"

size_t Tx::handler_for_read(uint8_t * resp_buf,
                uint8_t * resp_type,
                const uint8_t * req_buf,
                size_t req_len, void *arg) {
    Tx* tx_this = (Tx*)arg;
    
    DsReadReq * req = (DsReadReq *)req_buf;
    uint64_t obj_key = req->obj_key;

    DsObj * requested_obj = tx_this->data_store->lookup(obj_key);
    uint8_t * buf = resp_buf;

    tx_this->data_store->is_obj_locked(obj_key);
    if (tx_this->data_store->is_obj_locked(obj_key))
            (resp_type)[0] = DsRespType::READ_LOCKED;
    else (resp_type)[0] = DsRespType::READ_SUCCESS;

    int resp_size = ds_forge_read_resp(buf, DsRespType::READ_SUCCESS, requested_obj);
    return resp_size;
}

size_t Tx::handler_for_readnlock(uint8_t * resp_buf,
                uint8_t * resp_type,
                const uint8_t * req_buf,
                size_t req_len, void *arg) {
    Tx* tx_this = (Tx*)arg;
    
    DsReadReq * req = (DsReadReq *)req_buf;
    uint64_t obj_key = req->obj_key;

    DsObj * requested_obj = tx_this->data_store->lookup(obj_key);
    uint8_t * buf = resp_buf;

    if (tx_this->data_store->is_obj_locked(obj_key))
            (resp_type)[0] = DsRespType::READ_LOCKED;
    else (resp_type)[0] = DsRespType::READ_SUCCESS;

    int resp_size = ds_forge_read_resp(buf, DsRespType::RNL_SUCCESS, requested_obj);

    return resp_size;
}
