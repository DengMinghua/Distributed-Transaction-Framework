#ifndef MAPPINGS_H_
#define MAPPINGS_H_

#include <assert.h>
#include <stdint.h>
#include <cstdio>
#include <sys/types.h>
#include <unistd.h>
#include <cstdlib>
#include <sys/mman.h>
#include <fcntl.h>

#define DEFAULT_REGION_SIZE (1024 * 4096)
#define MAPPING_DEBUG 1
#define MAX_LOCAL_SIM_NODES 64
#define LOCAL_SIMULATION 1
#define MAX_END_POINT_SIZE 64

class Mappings {
private:
    uint8_t *region_ptr;

    uint8_t *local_sim_regions_ptr[MAX_LOCAL_SIM_NODES];
    char* end_points[MAX_END_POINT_SIZE];
    int node_id;

    int tot_num_nodes;
    int tot_num_primarys;

    int num_backups;
    int num_replicas;

    long memory_region_size;

    size_t file_size(int fd);

    void * setup_region_on_node(int node_id, const char * file = "");
public:
    Mappings(int node_id,
        int tot_num_nodes_,
        int tot_num_primarys_,
        int num_backups_,
        long memory_region_size_ = DEFAULT_REGION_SIZE
        );

    int get_primary(void* address);

    int get_backups_from_primary(int primary, int back_i);

    int get_backups(void* address, int back_i);

    int get_num_backups();
};

#endif
