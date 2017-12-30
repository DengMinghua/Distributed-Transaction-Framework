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

        RpcReq * read_req = rpc_client->new_req(read_item->rpc_type, read_item->primary_node, read_item->des, length);
        tx_rpc_req[r_index++] = read_req;

        size_t req_size = ds_forge_read_req(read_req, read_item->src, read_item->length);
        read_req->freeze();
    }

    for (size_t i = write_cnt; i < write_size; i++) {
        TxRwItem * write_item = write_set[i];

        RpcReq * write_req = rpc_client->new_req(write_item->rpc_type, write_item->primary_node, read_item->des, length);
        tx_rpc_req[r_index++] = write_req;

        size_t req_size = ds_forge_write_req(write_req, write_item->des, write_item->length, write_item->src);

        write_req->freeze();
    }
}

TxStatus Tx::commit() {

}

TxStatus Tx::validate


 
