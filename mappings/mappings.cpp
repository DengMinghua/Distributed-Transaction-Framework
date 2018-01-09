#include "mappings.h"

Mappings::Mappings(int node_id_, int tot_num_nodes_, int tot_num_primarys_, 
                int num_backups_, long memory_region_size_):
                node_id(node_id_), tot_num_nodes(tot_num_nodes_),
                tot_num_primarys(tot_num_primarys_), num_backups(num_backups_),
                memory_region_size(memory_region_size_), num_replicas(num_backups_ + 1) {
                    assert(tot_num_primarys < tot_num_nodes);
                    if (LOCAL_SIMULATION) {
                           for (int i = 0; i < tot_num_nodes; i++)
                                   local_sim_regions_ptr[i] = (uint8_t*) setup_region_on_node(i);
                    }
                    else {
                        region_ptr = (uint8_t *) setup_region_on_node(node_id);
                        // Add endpoint info here in network mode
                    }
                    
}

int Mappings::get_primary(void * offset) {
    assert((long)offset < memory_region_size * tot_num_primarys);
    return (((long)offset / memory_region_size) * num_replicas) % tot_num_nodes;
}

int Mappings::get_backups_from_primary(int primary, int back_i) {
    assert(primary >= 0 && primary < tot_num_primarys);
    assert(back_i >= 0 && back_i < num_backups);

    return ((primary + back_i + 1) % tot_num_nodes);
}

int Mappings::get_backups(void * offset, int back_i) {
   return get_backups_from_primary(get_primary(offset), back_i); 
}

int Mappings::get_num_backups() {
    return num_backups;
}

size_t Mappings::file_size(int fd) {
    off_t size = lseek(fd, 0, SEEK_END);
    return size;
}

void* Mappings::setup_region_on_node(int node_id, const char * file) {
#ifdef MAPPING_DEBUG
    printf("%s\n", __PRETTY_FUNCTION__);
#endif
    size_t size = memory_region_size * 
            (num_replicas * tot_num_primarys / tot_num_nodes);
    void * addr = NULL;
    int fd = -1;

    if (file != "") {
        fd = open(file, O_RDWR | O_NOATIME | O_CREAT, S_IRWXU);
        int f_size = file_size(fd);
        assert(size < f_size);
        addr = (void *)mmap(NULL, size, PROT_READ | PROT_WRITE,
                        MAP_SHARED, fd, 0);
#ifdef MAPPING_DEBUG
        printf("%d bytes in %s has been mapped to %ld\n",
                        (int)size, file, (long)addr);
#endif
    }
    else {
        addr = malloc(size);
#ifdef MAPPING_DEBUG
        printf("%d bytes of memory has been allocate to %ld\n",
                        (int)size, (long)addr);
#endif
    }
    return addr;
}
