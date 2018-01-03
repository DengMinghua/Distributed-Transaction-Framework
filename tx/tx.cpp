#include "tx.h"

void Tx::start() {
    assert(tx_status == TxStatus::COMMITTED ||
                    tx_status == TxStatus::ABORTED);
    
    tx_status = TxStatus::PROGRESSING;

    bzero(read_set, TX_MAX_READ_SET);
    bzero(write_set, TX_MAX_WRITE_SET);

    r_index = 0;
    w_index = 0;
    
#ifdef TX_DEBUG
    printf("%s\n",__PRETTY_FUNCTION__);
#endif
    
}

int Tx::add_to_read_set(TxRwAddress local_address, TxRwLength local_length, 
                TxRwAddress remote_address, TxRwLength remote_length) {
    assert(tx_status == TxStatus::PROGRESSING);
    assert(local_address != NULL && remote_address != NULL);

    TxRwItem item(local_address, local_length, 
                    remote_address, remote_length, TxRwMode::READ);

    assert(mappings != NULL);
    item.bind_primary(mappings);
    item.bind_backups(mappings);

    read_set[r_size++] = item;
    return item.primary();
}

int Tx::add_to_write_set(TxRwAddress local_address, TxRwLength local_length, 
                TxRwAddress remote_address, TxRwLength remote_length,
                TxRwMode mode_) {
    assert(tx_status == TxStatus::PROGRESSING);
    assert(local_address != NULL && remote_address != NULL);

    TxRwItem item(local_address, local_length,
                    remote_address, remote_length, mode_);

    assert(mappings != NULL);
    item.bind_primary(mappings);
    item.bind_backups(mappings);

    write_set[w_size++] = item;
    return item.primary();
}

TxStatus Tx::do_read() {
    assert(tx_status == TxStatus::PROGRESSING);

    assert(r_index <= r_size);
    assert(w_index <= w_size);
   
    rpc_client->clear_req_batch();


    for (size_t i = r_index; i < r_size; i++) {
        TxRwItem * read_item = &read_set[i];

        RpcReq * read_req = rpc_client->new_req(read_item->rpc_type, read_item->primary_node, 
                        (uint8_t*) read_item->local_address, read_item->local_length);
        tx_rpc_req[req_index++] = read_req;

        size_t req_size = ds_forge_read_req(read_req, DsReqType::DS_READ, (uint8_t *)read_item->remote_address, read_item->remote_length);
        read_req->freeze(req_size);
    }

    for (size_t i = w_index; i < w_size; i++) {
        TxRwItem * write_item = &write_set[i];

        RpcReq * write_req = rpc_client->new_req(write_item->rpc_type, write_item->primary_node, 
                        (uint8_t*)write_item->local_address, write_item->local_length);
        tx_rpc_req[req_index] = write_req;
        
        size_t req_size = ds_forge_read_req(write_req, DsReqType::DS_READNLOCK, (uint8_t*) write_item->remote_address, write_item->remote_length);

        write_req->freeze(req_size);
    }
    
    // Not yet implemented
    //rpc->send_reqs();
    
    int resp_cnt = 0;

    for (size_t i = r_index; i < r_size; i++) {
        TxRwItem * read_item = &read_set[i];
        DsRespType read_resp_type = (DsRespType) tx_rpc_req[resp_cnt]->resp_type;

        switch(read_resp_type) {
                case DsRespType::READ_SUCCESS:
                        read_item->local_length = tx_rpc_req[resp_cnt]->resp_len - sizeof(RpcMsgHdr);
                        read_item->done_read = true;
                        break;
                case DsRespType::READ_FAILED:
                        read_item->local_length = 0;
                        read_item->done_read = false;
                        break;
                case DsRespType::READ_LOCKED:
                        read_item->done_read = false;
                        tx_status = TxStatus::ABORTING;
                        break;
                default:
                        printf("Unknow response for read item requesting (%ld, %d) from node %d\n", 
                                        (long)read_item->remote_address, (int)read_item->remote_length,
                                        (int)read_item->primary_node);
        }
        resp_cnt++;
    }

    for (size_t i = w_index; i < w_size; i++) {
        TxRwItem * write_item = &write_set[i];
        DsRespType write_resp_type = (DsRespType) tx_rpc_req[resp_cnt]->resp_type;

        switch(write_resp_type) {
                case DsRespType::RNL_SUCCESS:
                        write_item->local_length = tx_rpc_req[resp_cnt]->resp_len - sizeof(RpcMsgHdr);
                        write_item->done_lock = true;
                        break;
                case DsRespType::RNL_FAILED:
                case DsRespType::RNL_LOCKED:
                        write_item->done_lock = false;
                        tx_status = TxStatus::ABORTING;
                        break;
                default:
                        printf("Unknow response for read item requesting (%ld, %d) from node %d\n",
                                         (long)write_item->remote_address, (int)write_item->remote_length,
                                         (int)write_item->primary_node);
        }

        resp_cnt++;
    }

    w_index = w_size;
    r_index = r_size;

    return tx_status;
}


TxStatus Tx::commit() {
    assert(tx_status == TxStatus::PROGRESSING);

    if (r_size > 0) {
        bool validation = validate();
        if (!validation) {
            abort();
            assert(tx_status == TxStatus::ABORTED);
            return TxStatus::ABORTED;
        }
    }

    if (w_size == 0) {
        tx_status = TxStatus::COMMITTED;
        return TxStatus::COMMITTED;
    }

    // rmsync() should be used to commit here



}

void Tx::abort() {
    assert(tx_status == TxStatus::PROGRESSING ||
                    tx_status == TxStatus::ABORTING);

    tx_status = TxStatus::ABORTED;

    rpc_client->clear_req_batch();

    size_t r_cnt = 0;

    for (size_t i = 0; i < w_size; i++) {
        TxRwItem * write_item = &write_set[i];
        if (!(write_item->done_lock)) continue;
        RpcReq * unlock_req = rpc_client->new_req(write_item->rpc_type,
                        write_item->primary_node, (uint8_t *)write_item->local_address, write_item->local_length);

        tx_rpc_req[i] = unlock_req;
        r_cnt++;

        size_t req_size = ds_forge_read_req(unlock_req, DsReqType::DS_UNLOCK, (uint8_t*)write_item->remote_address, write_item->remote_length);
        unlock_req->freeze(req_size);

    }
    // Not yet implemented
    //rpc_client->send_req();

    r_cnt = 0;

    for (size_t i = 0; i < w_size; i++) {
        TxRwItem * write_item = &write_set[i];
        if (!(write_item->done_lock)) continue;

        r_cnt++;
    }

}

bool Tx::validate() {
    rpc_client->clear_req_batch();

    for (size_t i = 0; i < r_index; i++) {
        TxRwItem * read_item = &read_set[i];

        if (read_item->done_read) {
               
        RpcReq * read_req = rpc_client->new_req(read_item->rpc_type, 
                        read_item->primary_node, (uint8_t*)read_item->local_address, read_item->local_length);
        tx_rpc_req[i] = read_req;

        size_t req_size = ds_forge_read_req(read_req, DsReqType::DS_READ, (uint8_t*)read_item->remote_address, read_item->remote_length);
        read_req->freeze(req_size);
        }
    }

    //rpc->send_reqs();

    for (size_t i = 0; i < r_index; i++) {
        TxRwItem * read_item = &read_set[i];

        DsRespType read_resp_type = (DsRespType) tx_rpc_req[i]->resp_type;

        if (read_resp_type == DsRespType::READ_SUCCESS) {
            
        }

        else return false;
    }

    return true;

}


 
