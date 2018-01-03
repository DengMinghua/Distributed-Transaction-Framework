#ifndef RPC_H_
#define RPC_H_
#include "rpc_def.h"
#include "rpc_types.h"

class Rpc {
private:
        size_t (*rpc_handler[RPC_TYPE_NUM]) (uint8_t* resp_buf,
                        uint8_t* resp_type,
                        const uint8_t* req_buf,
                        size_t req_len, void *arg)
                = {NULL};
        
        RpcReqBatch req_batch; 
public:
        void register_rpc_handler(int req_type,
                        size_t (*handler_) (uint8_t* resp_buf,
                                        uint8_t* resp_type,
                                        const uint8_t* req_buf,
                                        size_t req_len, void *arg),
                        void * arg);

        int required_recvs();
        ~Rpc();

        RpcReq * new_req(uint8_t req_type, int resp_node, uint8_t* resp_buf,
                        size_t max_resp_len) {
                
            int req_index = req_batch.num_reqs;

            RpcReq * req = &req_batch.reqs[req_index];
            req->resp_buf = resp_buf;
            req->max_resp_len = max_resp_len;
            req_batch.num_reqs++;

            int c_msg_index = -1;

            if (req_batch.c_msg_for_node[resp_node] >= 0) {
                c_msg_index = req_batch.c_msg_for_node[resp_node];
            }
            else {
                c_msg_index = req_batch.next_avaliable_c_msg_slot;
                req_batch.c_msg_for_node[resp_node] = c_msg_index;
                req_batch.next_avaliable_c_msg_slot++;

                req_batch.c_msg[c_msg_index].node_id = resp_node;
            }

            RpcCoalMsg * cmsg = &req_batch.c_msg[c_msg_index];

            RpcMsgHdr * cmsg_hdr = (RpcMsgHdr *) ((cmsg->req_buf).cur_ptr);
            cmsg_hdr->msg_type = req_type;
            cmsg->req_buf.cur_ptr += sizeof(RpcMsgHdr);

            req->req_buf = cmsg->req_buf.cur_ptr;
            req->cmsg_hdr = cmsg_hdr;
            req->cmsg_buf = &(cmsg->req_buf);

            return req;
        }

        Buffer* new_resp(int req_for_node, int req_index, uint32_t req_imm) {
            
        }

        void clear_req_batch() {
            req_batch.clear();
        }        

};

#endif
