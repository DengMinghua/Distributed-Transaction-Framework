#ifndef MAPPINGS_H_
#define MAPPINGS_H_
#include <assert.h>
#include <stdint.h>
class Mappings {
private:
    int tot_num_nodes;
    int tot_num_primarys;

    int num_backups;
    int num_replicas;
public:
    Mappings(int tot_num_nodes_,
        int tot_num_primarys_,
        int num_backups_):
        tot_num_nodes(tot_num_nodes_),
        tot_num_primarys(tot_num_primarys_),
        num_backups(num_backups_),
        num_replicas(num_backups_ + 1) {
            assert(num_replicas < tot_num_nodes);
        }

    int get_primary(TxRwAddress address) {
        return ((uint64_t)address % tot_num_primarys);
    }

    int get_backups_from_primary(int primary, int back_i) {
        assert(primary >= 0 && primary < tot_num_primarys);
        assert(back_i >= 0 && back_i < num_backups);

        return ((primary + back_i) % tot_num_nodes);
    }

    int get_backups(TxRwAddress address, int back_i) {
        return get_backups_from_primary(get_primary(address), back_i);
    }

    int get_num_backups() {
        return num_backups;
    }
};

#endif
