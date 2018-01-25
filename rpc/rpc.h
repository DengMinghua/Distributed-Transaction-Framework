#ifndef RPC_H_
#define RPC_H_
#include "rpc_def.h"
#include "rpc_types.h"
#include "../datastore/ds.h"
#include <pthread.h>
#include <unistd.h>


class Rpc {
private:
        size_t (*rpc_handler[RPC_TYPE_NUM]) (uint8_t* resp_buf,
                        uint8_t* resp_type,
                        const uint8_t* req_buf,
                        size_t req_len, void *arg)
                = {NULL};
        void * rpc_handler_arg[RPC_TYPE_NUM] = {NULL};

        RpcReqBatch req_batch;
        RpcRespBatch resp_batch;
        
        RpcServerStatus status;

        pthread_t server_tid;
        pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
        pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
        pthread_barrier_t barrier;
        bool send_signal = false;

        pthread_cond_t recv_cond = PTHREAD_COND_INITIALIZER;
        pthread_mutex_t recv_mtx = PTHREAD_MUTEX_INITIALIZER;
        bool recv_signal = false;

        uint16_t rpc_seq = 0;

public:
        void online();
        void offline();

        void register_rpc_handler(int req_type,
                        size_t (*handler_) (uint8_t* resp_buf,
                                        uint8_t* resp_type,
                                        const uint8_t* req_buf,
                                        size_t req_len, void *arg),
                        void * arg);
        
        void * local_sim_rpc_listener();
        static void* local_sim_rpc_listener_helper(void*);

        RpcReq * new_req(uint8_t req_type, int to_which_node, void* resp_buf,
                        size_t max_resp_len);

        Buffer* new_resp(int resp_to_whom);

        void clear_req_batch();
        
        // Because of the RDMA lib has not yet been finished,
        // This function is currently used only for local simulation
        void send_reqs();
        void send_resp();
        void recv_resp();


};

#endif
