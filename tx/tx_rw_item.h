#ifndef TX_RW_ITEM_H_
#define TX_RW_ITEM_H_

#include "tx_def.h"
#include "tx_conf.h"
#include "../mappings/mappings.h"

struct TxRwItem {
    TxRwMode rw_mode;
  
    TxRwAddress local_address;
    TxRwLength local_length;
  
    TxRwAddress remote_address;
    TxRwLength remote_length;

    TxRpcType rpc_type;

    int primary_node;
    int backup_nodes[TX_MAX_BACKUPS];

    bool done_read;
    bool done_lock;

    TxRwItem(TxRwAddress local_,
                TxRwLength local_length_,
                TxRwAddress remote_,
                TxRwLength remote_length_,
                TxRwMode mode_):
            local_address(local_),
            local_length(local_length_),
            remote_address(remote_),
            remote_length(remote_length_),
            rw_mode(mode_) {};
    TxRwItem(){};

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
