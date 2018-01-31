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

class Mappings {
        private:

                int node_id;

                int tot_num_nodes;
                int tot_num_primarys;

                int num_backups;
                int num_replicas;


        public:
                Mappings(int node_id,
                                int tot_num_nodes_,
                                int tot_num_primarys_,
                                int num_backups_
                        );

                //~Mappings();

                int get_primary(uint64_t address);

                int get_backups_from_primary(int primary, int back_i);

                int get_backups(uint64_t address, int back_i);

                int get_num_backups();
};

#endif
