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
    // status of current transaction 
    TxStatus tx_status;
    
    // a transaction should be bound with a memroy address mappings
    Mappings *mappings;
    
    // read set for read_only address
    TxRwItem read_set[TX_MAX_READ_SET];
    size_t r_size;
    size_t r_index;

    // write set will also be read from remote if needed
    TxRwItem write_set[TX_MAX_WRITE_SET];
    size_t w_size;
    size_t w_index;


    // RPC related, rpc_client is used for communication among machines
    Rpc *rpc_client;    
    
    // bookkeeping rpc request that related to current transaction
    RpcReq *tx_rpc_req[MAX_TX_RPC];
    size_t req_index;

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

    // start new transaction, must be called before add r/w set
    void start();
    
    // used to add address to r/w sets
    // will return a temp address for remote data that can be read/modified after do_read()
    // when given local_offset, remote data will be copy to specified local address
    void* add_to_write_set(void* remote_offset, size_t len, 
                    TxRwMode mode, void* local_offset = NULL);
    void* add_to_read_set(void* remote_offset, size_t len,
                    void* local_offset = NULL);
    
    // read specified data in r/w sets from remote
    TxStatus do_read();

    // commit change to remote replicas
    TxStatus commit();

    // validate the local/remote data version
    bool validate();

    // abort transaction
    void abort();

    bool log();


    // a handler for read data, now for testing use.
    static size_t handler_for_read(uint8_t * resp_buf,
                    uint8_t * resp_type,
                    const uint8_t * req_buf,
                    size_t req_len, void *arg);
};

#endif
