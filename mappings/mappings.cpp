#include "mappings.h"

Mappings::Mappings(int node_id_, int tot_num_nodes_, int tot_num_primarys_, 
	int num_backups_):
    node_id(node_id_), tot_num_nodes(tot_num_nodes_),
    tot_num_primarys(tot_num_primarys_), num_backups(num_backups_),
    memory_region_size(REGION_SIZE), num_replicas(num_backups_ + 1) {

	assert(tot_num_primarys < tot_num_nodes);

	if (LOCAL_SIMULATION) {
	    for (int i = 0; i < (tot_num_primarys * (num_backups + 1)); i++) {
            int node = i % tot_num_nodes;
            int level = i / tot_num_nodes;
            int region_id = i / (num_backups + 1);
		    local_sim_regions[node].region_ptrs[level] =
                (uint8_t*) setup_region_on_node(i);
            local_sim_regions[node].region_ids[level] = 
                region_id;
        }
	}
	else {
        int i = 0;
        while (i * (node_id + 1) <= tot_num_primarys * (num_backups + 1)) {
            regions.region_ptrs[i] = (uint8_t*) setup_region_on_node(node_id);
            regions.region_ids[i] = i * (node_id + 1) / (num_backups + 1);
            i++;
        }
	    //region_ptr = (uint8_t *) setup_region_on_node(node_id);
	    // Add endpoint info here in network mode
	}

	init_block_status();    
}

Mappings::~Mappings() {

    for (int i = 0 ; i < tot_num_nodes; i++){
        for (int j = 0; j < MAX_REGION_MANAGED_BY_ONE_NODE; j++)
	if (local_sim_regions[i].region_ptrs[j] != NULL)
	    free(local_sim_regions[i].region_ptrs[j]);
    }

    pthread_mutex_destroy(&block_status_lock);
}


int Mappings::get_primary(void * offset) {
    assert((long)offset < memory_region_size * tot_num_primarys);
    return (((long)offset / memory_region_size) * num_replicas) % tot_num_nodes;
}


int Mappings::get_backups_from_primary(int primary, int back_i) {
    return ((primary + back_i + 1) % tot_num_nodes);
}


int Mappings::get_backups(void * offset, int back_i) {
    return get_backups_from_primary(get_primary(offset), back_i); 
}

int Mappings::get_level_from_node(int node_id, void * address) {
    int region_id = (uint64_t)address / REGION_SIZE;
    for (int i = 0; i < MAX_REGION_MANAGED_BY_ONE_NODE; i++) {
        if (region_id == local_sim_regions[node_id].region_ids[i])
            return i;
    }
    return -1;
}

int Mappings::get_num_backups() {
    return num_backups;
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


void Mappings::init_block_status() {
   
    pthread_mutex_init(&(block_status_lock),NULL);
    
    for (int i = 0; i < NUM_BLOCKS; i++) {
	block_status[i].is_lock = false;
	block_status[i].version = 0;
    }
}


bool Mappings::check_blocks(int l, int r) {
    
    assert(l >= 0 && r < NUM_BLOCKS);
    
    pthread_mutex_lock(&block_status_lock);
    
    for (int i = l; i < r; i++) {
	if (block_status[i].is_lock) {
	    pthread_mutex_unlock(&block_status_lock);
	    return false;
	}
    
    }
    pthread_mutex_unlock(&block_status_lock);
    
    return true;
}


bool Mappings::lock_blocks(int l, int r) {
    
    assert(l >= 0 && r < NUM_BLOCKS);
    
    pthread_mutex_lock(&block_status_lock);
    
    for (int i = l; i < r; i++) {
	if (block_status[i].is_lock) {
	    pthread_mutex_unlock(&block_status_lock);
	    return false;
	}
	else {
	    block_status[i].is_lock = true;
	}
    }
    
    pthread_mutex_unlock(&block_status_lock);
    
    return true;
}


bool Mappings::unlock_blocks(int l, int r) {
    
    assert(l >= 0 && r < NUM_BLOCKS);

    pthread_mutex_lock(&block_status_lock);
    
    for (int i = l; i < r; i++) 
	block_status[i].is_lock = true;
    
    pthread_mutex_unlock(&block_status_lock);
    
    return true;
}


uint8_t Mappings::get_block_version(int i) {
    
    assert(i >= 0 && i < NUM_BLOCKS);
    
    return block_status[i].version;
}


void Mappings::get_blocks_version(int l, int r, uint8_t* version) {
    
    for (int i = 0; i < r - l; i++) {
	version[i] = get_block_version(i);
    }
}

size_t file_size(int fd) {
    
    off_t size = lseek(fd, 0, SEEK_END);
    return size;
}
