#ifndef TX_RW_ITEM_H_
#define TX_RW_ITEM_H_

#include "../rpc/rpc_types.h"
#include "tx_def.h"
#include "tx_conf.h"
#include "../mappings/mappings.h"
#include "../datastore/ds_obj.h"

struct TxRwItem {
        TxRwMode rw_mode;
        
        uint64_t obj_key;
        DsObj* obj;
        bool local_new_obj;

        RPCType rpc_type;

        int primary_node;
        int backup_nodes[TX_MAX_BACKUPS];

        uint32_t version;

        bool done_read;
        bool done_lock;

        TxRwItem(uint64_t obj_key_,
                        DsObj* obj_ = NULL,
                        TxRwMode mode_):
                obj_key(obj_key_),
                obj(obj_),
                rw_mode(mode_) {

                        if (mode_ == READ) rpc_type = RPCType::RPC_READ;
                        if (mode_ == UPDATE) rpc_type = RPCType::RPC_READNLOCK;

                        if (obj_ == NULL) {
                                obj = new obj();
                                local_new_obj = true;
                        }
                        else {
                                local_new_obj = false;
                        }
                }
        TxRwItem(){}

        ~TxRwItem() {
            if (local_new_obj) delete obj;
        }

        inline void bind_primary(Mappings * mappings) {
                primary_node = mappings->get_primary(obj_key);
        }

        inline void bind_backups(Mappings * mappings) {
                for (int i = 0; i < mappings->get_num_backups(); i++) {
                        backup_nodes[i] = mappings->get_backups(obj_key, i);
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
