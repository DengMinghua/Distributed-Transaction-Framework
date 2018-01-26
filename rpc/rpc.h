#ifndef RPC_H_
#define RPC_H_
#include "rpc_def.h"
#include "rpc_types.h"
#include "../datastore/ds.h"
#include <pthread.h>
#include <unistd.h>


class Rpc {
private:
        // Rpc handler vector
        size_t (*rpc_handler[RPC_TYPE_NUM]) (uint8_t* resp_buf,
                        uint8_t* resp_type,
                        const uint8_t* req_buf,
                        size_t req_len, void *arg)
                = {NULL};
        // parameter vector for different rpc handlers
        void * rpc_handler_arg[RPC_TYPE_NUM] = {NULL};

        // request batch for client end 
        RpcReqBatch req_batch;
        // resp batch for server end
        RpcRespBatch resp_batch;
        
        //  server status
        RpcServerStatus status;
        
        // pthread stuff for local thread simulation 
        pthread_t server_tid;
        pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
        pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
        pthread_barrier_t barrier;
        bool send_signal = false;

        pthread_cond_t recv_cond = PTHREAD_COND_INITIALIZER;
        pthread_mutex_t recv_mtx = PTHREAD_MUTEX_INITIALIZER;
        bool recv_signal = false;

        // an autoincresed sequence for all the request create by this rpc class

        uint16_t rpc_seq = 0;

public:
        // make the server end online
        void online();
        // make the server end offline
        void offline();

        // register rpc handler for specific req type
        // will be trigered in server end 
        void register_rpc_handler(int req_type,
                        size_t (*handler_) (uint8_t* resp_buf,
                                        uint8_t* resp_type,
                                        const uint8_t* req_buf,
                                        size_t req_len, void *arg),
                        void * arg);
        
        // a thread function that play as an local server for testing
        void * local_sim_rpc_listener();
        static void* local_sim_rpc_listener_helper(void*);

        
        // create an new request, with giving request type, remote node id,
        // response buffer for future response and max response length that the buffer can hold 
        RpcReq * new_req(uint8_t req_type, int to_which_node, void* resp_buf,
                        size_t max_resp_len);

        // create an new response to the node who send request
        Buffer* new_resp(int resp_to_whom);

        // clear request batch, should be called after a round trip had finished.
        void clear_req_batch();
        
        // Because of the RDMA lib has not yet been finished,
        // These function is currently used only for local simulation
        void send_reqs();
        void send_resp();
        void recv_resp();


};

#endif
