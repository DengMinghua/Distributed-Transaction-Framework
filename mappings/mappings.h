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
#include <cstring>
#include <pthread.h>


#define REGION_SIZE (1024 * 4096)
#define BLOCK_SIZE 64
#define NUM_BLOCKS (REGION_SIZE / BLOCK_SIZE) * 8

#define MAPPING_DEBUG 1
#define MAX_LOCAL_SIM_NODES 64

#define LOCAL_SIMULATION 1

#define MAX_END_POINT_SIZE 64

#define MAX_REGION_MANAGED_BY_ONE_NODE 8

static size_t file_size(int fd);


struct BlockStatus {
    bool is_lock = false;
    uint8_t version = 0;
};

struct RegionGroup {
    uint8_t* region_ptrs[MAX_REGION_MANAGED_BY_ONE_NODE];
    int region_ids[MAX_REGION_MANAGED_BY_ONE_NODE];
};

class Mappings {

    private:

	int node_id;

	int tot_num_nodes;
	int tot_num_primarys;

	int num_backups;
	int num_replicas;


	long memory_region_size;
	RegionGroup regions;
	RegionGroup local_sim_regions[MAX_LOCAL_SIM_NODES];
	char* end_points[MAX_END_POINT_SIZE];

	BlockStatus block_status[NUM_BLOCKS];
	pthread_mutex_t block_status_lock;

	void * setup_region_on_node(int node_id, const char * file = "");
    
    public:
	Mappings(int node_id,
		int tot_num_nodes_,
		int tot_num_primarys_,
		int num_backups_
		);

	~Mappings();

	int get_primary(void* address);

	int get_backups_from_primary(int primary, int back_i);

	int get_backups(void* address, int back_i);

	int get_num_backups();

    int get_level_from_node(int node_id, void* address);

	void init_block_status();

	bool check_blocks(int l, int r);

	bool lock_blocks(int l, int r);

	bool unlock_blocks(int l, int r);

	uint8_t get_block_version(int i);

	void get_blocks_version(int l, int r, uint8_t *); 

	inline void* local_sim_get_value(void* address) {

#ifndef LOCAL_SIMULATION
	    assert(0);
#endif
        int node = get_primary(address);
        int level = get_level_from_node(node, address);
	    uint8_t* ptr =  local_sim_regions[node].region_ptrs[level];
	    ptr += (uint64_t)address % memory_region_size;
	    return ptr;
	}

	inline void local_sim_put_value(void* address, void* value, size_t len) {

#ifndef LOCAL_SIMULATION
	    assert(0);
#endif
        int node = get_primary(address);
        int level = get_level_from_node(node, address);
	    uint8_t* ptr =  local_sim_regions[node].region_ptrs[level];
	    ptr += (uint64_t)address % memory_region_size;

	    memcpy(ptr, value, len);
	}
};

#endif
