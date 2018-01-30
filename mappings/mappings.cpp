#include "mappings.h"

Mappings::Mappings(int node_id_, int tot_num_nodes_, int tot_num_primarys_, 
                int num_backups_):
                node_id(node_id_), tot_num_nodes(tot_num_nodes_),
                tot_num_primarys(tot_num_primarys_), num_backups(num_backups_),
                num_replicas(num_backups_ + 1) 
}

int Mappings::get_primary(uint64_t hash_key) {
    assert(hash_key < memory_region_size * tot_num_primarys);
    return ((hash_key / memory_region_size) * num_replicas) % tot_num_nodes;
}

int Mappings::get_backups_from_primary(int primary, int back_i) {
    return ((primary + back_i + 1) % tot_num_nodes);
}

int Mappings::get_backups(uint64_t hash_key, int back_i) {
   return get_backups_from_primary(get_primary(hash_key), back_i); 
}

int Mappings::get_num_backups() {
    return num_backups;
}
