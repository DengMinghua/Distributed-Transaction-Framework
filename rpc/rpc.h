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
        ~Rpc() {};

        RpcReq * new_req(uint8_t req_type, int resp_node, uint8_t* resp_buf,
                        size_t max_resp_len) {
            #ifdef RPC_DEBUG
                printf("%s\n",__PRETTY_FUNCTION__);
            #endif
                
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
            (cmsg->num)++;
        
            #ifdef RPC_DEBUG
                //printf("%s\n",__PRETTY_FUNCTION__);
                printf("\tNew request started:\n\treq type:\t%d\n\treq buff addr:\t%ld\n\tresp node:\t%d\n\tresp buff addr:\t%ld\n\tmax resp len:\t%d\n",
                                (int)req_type, (long)req->req_buf,
                                (int)resp_node, (long)resp_buf,
                                (int)max_resp_len);
            #endif

            return req;
        }

        Buffer* new_resp(int resp_to_whom, int num_reqs, uint32_t req_imm) {
            RpcCoalMsg * c_msg = &resp_batch.c_msg[resp_batch.num_c_msg++];
            c_msg->node_id = resp_to_whom;
            c_msg->num = num_reqs;
            return &(c_msg->resp_buf);
        }

        void clear_req_batch() {
            req_batch.clear();
        } 
        
        // Because of the RDMA lib has not yet been finished,
        // This function is currently used only to print reqs for debugging
        void send_reqs() {
#ifdef RPC_DEBUG
           printf("%s\n", __PRETTY_FUNCTION__);
           RpcReqBatch * batch = &req_batch;
           for (int i = 0; i < MAX_NODES; i++) {
                if (batch->c_msg_for_node[i] != -1) {
                    int index = batch->c_msg_for_node[i];
                    RpcCoalMsg * msg = &((batch->c_msg)[index]);
                    printf("Request for node %d\n", i);
                    printf("---------------MSG BUF PRETTY PRINT---------------\n");
                    printf("uint_8 node_id: \t%d\n", msg->node_id);
                    printf("uint_8 num: \t%d\n", msg->num);
                    printf("Buffer req_buf:\n");
                    uint8_t  * ptr = (msg->req_buf).head_ptr;
                    for (int j = 0; j < msg->num; j++) {
                        RpcMsgHdr* hdr = (RpcMsgHdr*) ptr;
                        printf("uint8_t msg_type:\t%d\n", hdr->msg_type);
                        printf("uint16_t msg_size:\t%d\n", hdr->size);
                        ptr += sizeof(RpcMsgHdr);
                        DsReadReq * req = (DsReadReq *) ptr;
                        printf("uint32_t req_type:\t%ld\n", (long)(req->req_type));
                        printf("uint8_t * req_type:\t%ld\n", (long)(req->address));
                        printf("uint32_t length:\t%ld\n", (long)(req->length));
                        ptr += sizeof(DsReadReq);
                    }
                printf("-------------------MSG BUF END-------------------\n");
                }
           }
#endif
        }

};

#endif
