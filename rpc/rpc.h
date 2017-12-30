#ifndef RPC_H_
#define RPC_H_
#include "rpc_def.h"

class Rpc {
private:
        size_t (*rpc_handler[RPC_TYPE_NUM]) (RespBuf* resp_buf,
                        MsgType* resp_type,
                        const ReqBuf* req_buf,
                        size_t req_len, void *arg)
                = {NULL};
        
        RpcReqBatch req_batch;

        int rpc_port;
        struct hrd_ctrl_blk *cb = NULL;
        uint32_t lkey;

        struct ibv_ah *ah[MAX_NODES] = {NULL};
        int rem_qpn[MAX_NODES] = {0};

        int active_qp = 0;
        int nb_pending[RPC_MAX_QPS];
        struct ibv_send_wr send_wr[RPC_MAX_POSTLIST + 1];
        struct ibv_sge send_sgl[RPC_MAX_POSTLIST];

        size_t recv_step = -1;
        int recv_head = 0;
        int recv_slack = 0;
        int recvs_to_post = 0;

        struct ibv_recv_wr recv_wr[HRD_RQ_DEPTH];
        struct ibv_sge recv_sgl[HDR_RQ_DEPTH];
        struct ibv_wc wc[HRD_RQ_DEPTH];

public:
        void register_rpc_handler(int req_type,
                        size_t (*handler_) (RespBuf* resp_buf,
                                        MsgType* resp_type,
                                        const ReqBuf* req_buf,
                                        size_t req_len, void *arg),
                        void * arg);

        int required_recvs();
        ~Rpc();

        RpcReq * new_req(MsgType req_type, int resp_node, RespBuf*,
                        size_t max_resp_len) {
                
            int req_index = req_batch.num_reqs;

            RpcReq * req = req_bacth.reqs[req_index];
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

            RpcCoalMsg * cmsg = req_batch.c_msg[c_msg_index];

            RpcMsgHdr * cmsg_hdr = (RpcReqHdr *) cmsg->req_buf.cur_ptr;
            cmsg_hdr->req_type = req_type;
            cmsg->req_buf.cur_ptr += sizeof(RpcReqHdr);

            req->req_buf = cmsg->req_buf.cur_ptr;
            req->cmsg_hdr = cmsg_hrd;
            req->cmsg_buf = cmsg->req_buf;

            return req;
        }

        Buffer* new_resp(int req_for_node, int req_index, uint32_t req_imm) {
            
        }

        void clear_req_batch() {
            req_batch.clear();
        }

        







}
