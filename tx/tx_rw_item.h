#ifndef TX_RW_ITEM_H_
#define TX_RW_ITEM_H_

#include "tx_def.h"
#include "tx_conf.h"
#include "mappings.h"

class TxRwItem {
private:
    TxRwMode rw_mode;
  
    TxRwAddress src;
    TxRwLength src_length;
  
    TxRwAddress des;
    TxRwLength des_length

    TxRpcType rpc_type;

    int primary_node;
    int backup_nodes[TX_MAX_BACKUPS];

    bool done_read;
    bool done_lock;

public:
    TxRwItem(TxRwAddress src_,
                TxRwAddress des_,
                TxRwLength length_,
                TxRwMode mode_):
            src(src_),
            des(des_),
            length(length_),
            rw_mode(mode_);

    inline void bind_primary(Mappings * mappings) {
        primary_node = mappings->get_primary(src);
    }

    inline void bind_backups(Mappings * mappings) {
        for (int i = 0; i < mappings->num_backups; i++) {
            backup_nodes[i] = mappings->get_backups(src, i);
        }
    }

    inline int primary() {
        return primary;
    }

    inline int backups(int i) {
        return backups[i];
    }
}

#endif
