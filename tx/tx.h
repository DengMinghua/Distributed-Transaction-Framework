#ifndef TX_H_
#define TX_H_

#include "tx_conf.h"
#include "tx_def.h"
#include "../mappings/mappings.h"
#include "tx_rw_item.h"

#include "../rpc/rpc.h"

#include "../datastore/ds.h"

#include <assert.h>

class Tx {
private:
    TxStatus tx_status;
    Mappings *mappings;
    
    TxRwItem read_set[TX_MAX_READ_SET];
    size_t r_size;
    size_t r_index;

    TxRwItem write_set[TX_MAX_WRITE_SET];
    size_t w_size;
    size_t w_index;

    Rpc *rpc_client;    

    RpcReq *tx_rpc_req[MAX_TX_RPC];
    size_t req_index;

    uint8_t * local_log_buffer;
public:
    Tx(Mappings * mappings_, Rpc * rpc_client_):
            mappings(mappings_), rpc_client(rpc_client_),
            tx_status(TxStatus::COMMITTED),
            r_size(0), r_index(0),
            w_size(0), w_index(0) {};
    
    ~Tx() {
            assert(tx_status == TxStatus::COMMITTED ||
                            tx_status == TxStatus::ABORTED);
            delete rpc_client;
    }
    void start();
    
    TxRwAddress add_to_write_set(TxRwAddress remote_offset, TxRwLength len, 
                    TxRwMode mode, TxRwAddress local_offset = NULL);
    TxRwAddress add_to_read_set(TxRwAddress remote_offset, TxRwLength len,
                    TxRwAddress local_offset = NULL);
    
    TxStatus do_read();

    TxStatus commit();

    bool validate();

    void abort();

    bool log();
};

#endif
