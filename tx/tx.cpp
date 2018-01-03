#include "tx.h"

void Tx::start() {
    tx_assert(tx_status == TxStatus::COMMITTED ||
                    tx_status == TxStatus::ABORTED);
    
    tx_status = TxStatus::PROGRESSING;

    bzero(read_set, TX_MAX_READ_SET);
    bzero(write_set, TX_MAX_WRITE_SET);

    r_index = 0;
    w_index = 0;
}

int Tx::add_to_read_set(TxRwAddress src, TxRwAddress des, TxRwLength len) {
    tx_assert(tx_status == TxStatus::PROGRESSING);
    tx_assert(src != NULL && des != NULL);

    TxRwItem item(src, des, len, TxRwMode::READ);

    tx_assert(mappings != NULL);
    item.bind_primary(mappings);
    item.bind_backups(mappings);

    read_set[read_size++] = item;
    return item.primary();
}

int Tx::add_to_write_set(TxRwAddress src, TxRwAddress des, TxRwLength len,
                TxRwMode mode) {
    tx_assert(tx_status == TxStatus::PROGRESSING);
    tx_assert(src != NULL && des != NULL);

    TxRwItem item(src, des, len, mode);

    tx_assert(mappings != NULL);
    item.bind_primary(mappings);
    item.bind_backups(mappings);

    write_set[write_size++] = item;
    return item.primary();
}

TxStatus Tx::do_read() {
    tx_assert(tx_status == TxStatus::PROGRESSING);

    tx_assert(read_cnt <= read_size);
    tx_assert(write_cnt <= write_size);
   
    rpc_client->clear_req_batch();


    for (size_t i = read_cnt; i < read_size; i++) {
        TxRwItem * read_item = read_set[i];

        RpcReq * read_req = rpc_client->new_req(read_item->rpc_type, read_item->primary_node, read_item->des, des_length);
        tx_rpc_req[req_index++] = read_req;

        size_t req_size = ds_forge_read_req(read_req, DsReqType::READ, read_item->src, read_item->src_length);
        read_req->freeze();
    }

    for (size_t i = write_cnt; i < write_size; i++) {
        TxRwItem * write_item = write_set[i];

        RpcReq * write_req = rpc_client->new_req(write_item->rpc_type, write_item->primary_node, read_item->src, src_length);
        tx_rpc_req[req_index] = write_req;
        
        size_t req_size = ds_forge_read_req(write_req, DsReqType::READNLOCK, write_item->des, write_item->des_length);

        write_req->freeze();
    }

    rpc->send_reqs();
    
    int resp_cnt = 0;

    for (size_t i = read_cnt; i < read_size; i++) {
        TxRwItem * read_item = &read_set[i];
        DsRespType read_resp_type = (DsRespType) tx_rpc_req[resp_cnt]->resp_type;

        switch(read_resp_type) {
                case DsRespType::READ_SUCCESS:
                        read_item->des_length = tx_rpc_req[resp_cnt]->resp_len - sizeof(RpcMsgHdr);
                        done_read = true;
                        break;
                case DsRespType::READ_FAILED:
                        read_item->des_length = 0;
                        done_read = false;
                        break;
                case DsRespType::READ_LOCKED:
                        done_read = false;
                        tx_status = TxStatus::ABORTING;
                        break;
                default:
                        printf("Unknow response for read item requesting (%ld, %d) from node %d\n", 
                                        (long)read_item->src, (int)read_item->src_length,
                                        (int)read_item->primary_node);
        }
        resp_cnt++;
    }

    for (size_t i = write_cnt; i < write_size; i++) {
        TxRwItem * write_item = &write_set[i];
        DsRespType write_resp_type = (DsRespType) tx_rpc_req[resp_cnt]->resp_type;

        switch(write_resp_type) {
                case DsRespType::RNL_SUCCESS:
                        read_item->src_length = tx_rpc_req[resp_cnt]->resp_len - sizeof(RpcMsgHdr);
                        done_lock = true;
                        break;
                case DsRespType::RNL_FAILED:
                case DSRespType::RNL_LOCKED:
                        done_lock = false;
                        tx_status = TxStatus::ABORTING;
                        break;
                default:
                        printf("Unknow response for read item requesting (%ld, %d) from node %d\n",
                                         (long)read_item->src, (int)read_item->src_length,
                                         (int)read_item->primary_node);
        }

        resp_cnt++;
    }

    write_cnt = write_size;
    read_cnt = read_size;

    return tx_status;
}


TxStatus Tx::commit() {
    tx_assert(tx_status == TxStatus::PROGRESSING);

    if (read_size > 0) {
        bool validation = validate();
        if (!validation) {
            abort();
            tx_assert(tx_status == TxStatus::ABORTED);
            return TxStatus::ABORTED;
        }
    }

    if (write_size == 0) {
        tx_status = TxStatus::COMMITED;
        return TxStatus::COMMITED;
    }

    // rmsync() should be used to commit here



}

void Tx::abort() {
    tx_assert(tx_status == TxStatus::PROGRESSING ||
                    tx_status == TxStatus::ABORTING);

    tx_status = TxStatus::ABORTED;

    rpc_client->clear_req_batch();

    size_t r_cnt = 0;

    for (size_t i = 0; i < write_size; i++) {
        TxRwItem * write_item = &write_set[i];
        if (!(write_item->done_lock)) continue;
        RpcReq * unlock_req = rpc_client->new_req(write_item->rpc_type,
                        write_item->primary_node, write_item->des, write_item->des_length);

        tx_rpc_req[i] = write_req;
        r_cnt++;

        size_t req_size = ds_forge_read_req(unlock_req, DsReqType::UNLOCK, write_item->des, write_item->des_length);
        unlock_req->freeze();
    }

    rpc_client->send_req();

    r_cnt = 0;

    for (size_t i = 0; i < write_size; i++) {
        TxRwItem * write_item = &write_set[i];
        if (!(write_item->done_lock)) continue;

        r_cnt++;
    }

}

bool Tx::validate() {
    rpc_client->clear_req_batch();

    for (size_t i = 0; i < read_cnt; i++) {
        TxRwItem * read_item = &read_set[i];

        if (read_item->done_read) {
               
        RpcReq * read_req = rpc_client->new_req(read_item->rpc_type, 
                        read_item->primary_node, read_item->des, read_item->des_length);
        tx_rpc_req[i] = read_req;

        size_t req_size = ds_forge_read_req(read_req, DsReqType::READ, read_item->src, read_item->src_length);
        read_req->freeze();
        }
    }

    rpc->send_reqs();

    for (size_t i = 0; i < read_cnt; i++) {
        TxRwItem * read_item = &read_set[i];

        DsRespType read_resp_type = (DsRespType) tx_rpc_req[i]->resp_type;

        if (read_resp_type == DsRespType::READ_SUCCESS) {
            
        }

        else return false;
    }

    return true;

}


 
