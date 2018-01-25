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
    uint8_t * buf = resp_buf;
    uint8_t block_version[MAX_BLOCKS_PER_REQUEST];
    tx_this->mappings->get_blocks_version(address / BLOCK_SIZE, (address + len) / BLOCK_SIZE + 1, block_version);
    int blocks = len / BLOCK_SIZE + 1;
    int resp_size = ds_forge_read_resp(buf, DsRespType::READ_SUCCESS, value, len, blocks, block_version);
    (resp_type)[0] = DsRespType::READ_SUCCESS;
    return resp_size;
}
