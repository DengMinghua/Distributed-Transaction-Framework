#ifndef RPC_H_
#define RPC_H_
#include "rpc_def.h"
#include "rpc_types.h"
#include "../datastore/ds.h"
class Rpc {
private:
        size_t (*rpc_handler[RPC_TYPE_NUM]) (uint8_t* resp_buf,
                        uint8_t* resp_type,
                        const uint8_t* req_buf,
                        size_t req_len, void *arg)
                = {NULL};
        
        RpcReqBatch req_batch;
        RpcRespBatch resp_batch;
public:
        void register_rpc_handler(int req_type,
                        size_t (*handler_) (uint8_t* resp_buf,
                                        uint8_t* resp_type,
                                        const uint8_t* req_buf,
                                        size_t req_len, void *arg),
                        void * arg);

        int required_recvs();
        ~Rpc();

        RpcReq * new_req(uint8_t req_type, int to_which_node, uint8_t* resp_buf,
                        size_t max_resp_len);

        Buffer* new_resp(int resp_to_whom, int num_reqs, uint32_t req_imm);

        void clear_req_batch();
        
        // Because of the RDMA lib has not yet been finished,
        // This function is currently used only to print reqs for debugging
        void send_reqs();
};

#endif
