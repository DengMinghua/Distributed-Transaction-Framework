#ifndef TX_RW_ITEM_H_
#define TX_RW_ITEM_H_

#include "../rpc/rpc_types.h"
#include "tx_def.h"
#include "tx_conf.h"
#include "../mappings/mappings.h"
#include "../datastore/ds.h"

struct TxRwItem {
        TxRwMode rw_mode;

        void* local_address;
        size_t local_length;

        bool local_malloc;
        void* remote_address;
        size_t remote_length;

        int num_blocks;
        uint8_t block_version[MAX_BLOCKS_PER_REQUEST];

        RPCType rpc_type;

        int primary_node;
        int backup_nodes[TX_MAX_BACKUPS];

        bool done_read;
        bool done_lock;

        //uint8_t item_resp_buffer[MAX_ITEM_RESP_BUF];

        TxRwItem(void* remote_offset_,
                        size_t len_,
                        void* local_offset_,
                        TxRwMode mode_):
                local_length(len_),
                remote_address(remote_offset_),
                remote_length(len_),
                rw_mode(mode_) {

                        if (mode_ == READ) rpc_type = (RPCType)DsReqType::DS_READ; 
                        if (mode_ == UPDATE) rpc_type = (RPCType)DsReqType::DS_READNLOCK;

                        if (local_offset_ == NULL) {
                                local_address = (void*)malloc(local_length);
                                local_malloc = true;
                        }
                        else {
                                local_malloc = false;
                        }
                }
        TxRwItem(){}

        inline void bind_primary(Mappings * mappings) {
                primary_node = mappings->get_primary(remote_address);
        }

        inline void bind_backups(Mappings * mappings) {
                for (int i = 0; i < mappings->get_num_backups(); i++) {
                        backup_nodes[i] = mappings->get_backups(remote_address, i);
                }
        }

        inline int primary() {
                return primary_node;
        }

        inline int backups(int i) {
                return backup_nodes[i];
        }
};

#endif
