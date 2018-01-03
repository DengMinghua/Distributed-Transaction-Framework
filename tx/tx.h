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
public:
    Tx(Mappings * mappings_):
            mappings(mappings_) {};
    void start();
    
    int add_to_write_set(TxRwAddress local_addr, TxRwLength local_len, 
                    TxRwAddress remote_addr, TxRwLength remote_len,
                    TxRwMode mode);
    int add_to_read_set(TxRwAddress local_addr, TxRwLength local_len,
                    TxRwAddress remote_addr, TxRwLength remote_len);
    
    TxStatus do_read();

    TxStatus commit();

    bool validate();

    void abort();
};

#endif
