#include "tx.h"

size_t Tx::handler_for_read(uint8_t * resp_buf,
                uint8_t * resp_type,
                const uint8_t * req_buf,
                size_t req_len, void *arg) {
    DsReadReq * req = (DsReadReq *)req_buf;
    long address = (long)req->address;
    char * value =(char*) ((Tx*)arg)->mappings->local_sim_get_value(address);
    printf("the value for address %ld is %s\n",address, value);
    return 0;
}
