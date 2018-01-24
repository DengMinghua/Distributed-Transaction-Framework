#include "tx.h"

size_t Tx::handler_for_read(uint8_t * resp_buf,
                uint8_t * resp_type,
                const uint8_t * req_buf,
                size_t req_len, void *arg) {
    Tx* tx_this = (Tx*)arg;
    DsReadReq * req = (DsReadReq *)req_buf;
    long address = (long)req->address;
    size_t len = req->length;
    uint8_t * value = tx_this->mappings->local_sim_get_value(address);
    Buffer *buf = (Buffer*)resp_buf;
    int resp_buf_size = ds_forge_read_resp(buf, DsRespType::READ_SUCCESS, (uint8_t*)address, value, len);
    buf->cur_ptr += resp_buf_size;

    return 0;
}
