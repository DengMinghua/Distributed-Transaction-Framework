#ifndef TX_RW_ITEM_H_
#define TX_RW_ITEM_H_

#include "tx_def.h"
#include "tx_conf.h"
#include "../mappings/mappings.h"

struct TxRwItem {
    TxRwMode rw_mode;
  
    TxRwAddress local_address;
    TxRwLength local_length;
  
    bool local_malloc;
    TxRwAddress remote_address;
    TxRwLength remote_length;

    TxRpcType rpc_type;

    int primary_node;
    int backup_nodes[TX_MAX_BACKUPS];

    bool done_read;
    bool done_lock;

    TxRwItem(TxRwAddress remote_offset_,
                TxRwLength len_,
                TxRwAddress local_offset_,
                TxRwMode mode_):
            local_length(len_),
            remote_address(remote_offset_),
            remote_length(len_),
            rw_mode(mode_) {
                    if (local_offset_ == NULL) {
                local_address = (TxRwAddress)malloc(local_length);
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
