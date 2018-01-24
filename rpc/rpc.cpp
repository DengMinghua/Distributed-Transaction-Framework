#include "rpc.h"

void* Rpc::local_sim_rpc_listener() {
        printf("%s\n", __PRETTY_FUNCTION__);
        printf("Local Simlulation Server UP!\n");
        pthread_barrier_wait(&barrier);
        while (status == ONLINE) {
                pthread_mutex_lock(&mtx);
                pthread_cond_wait(&cond, &mtx);
                if (status == SHUTING_DOWN) {
                        status = OFFLINE;
                        pthread_mutex_unlock(&mtx);
                        break;
                }
                RpcReqBatch * batch = &req_batch;
                for (int i = 0; i < MAX_NODES; i++) {
                        if (batch->c_msg_for_node[i] != -1) {
                                int index = batch->c_msg_for_node[i];
                                RpcCoalMsg * msg = &((batch->c_msg)[index]);
                                printf("Recieved Message for node %d\n", i);
                                uint8_t  * ptr = (msg->req_buf).head_ptr;
                                Buffer * buffer = new_resp(i, msg->num, 0);
                                for (int j = 0; j < msg->num; j++) {
                                        RpcMsgHdr* hdr = (RpcMsgHdr*) ptr;
                                        int rpc_type = hdr->msg_type;
                                        size_t rpc_size = hdr->size;
                                        ptr += sizeof(RpcMsgHdr);
                                        DsReadReq * req = (DsReadReq *) ptr;
                                        rpc_handler[rpc_type]((uint8_t*) buffer, (uint8_t*)&rpc_type, (uint8_t*)req, sizeof(DsReadReq), rpc_handler_arg[rpc_type]);
                                        /*
                                           printf("uint32_t req_type:\t%ld\n", (long)(req->req_type));
                                           printf("uint8_t * req_address:\t%ld\n", (long)(req->address));
                                           printf("uint32_t length:\t%ld\n", (long)(req->length));
                                         */
                                        ptr += sizeof(DsReadReq);
                                }
                        }
                }

                printf("server sensed\n");
                pthread_barrier_wait(&barrier);
                pthread_mutex_unlock(&mtx);
        }
        status = OFFLINE;
}

void * Rpc::local_sim_rpc_listener_helper(void* This) {
        return ((Rpc *)This)->local_sim_rpc_listener();
}

void Rpc::online() {
#ifdef LOCAL_SIM
        status = ONLINE;
        pthread_barrier_init(&barrier, NULL, 2);
        int ret = pthread_create(&server_tid, NULL, local_sim_rpc_listener_helper, this);
        pthread_barrier_wait(&barrier);
        pthread_barrier_destroy(&barrier);
#endif
}

void Rpc::offline() {
#ifdef LOCAL_SIM
        pthread_mutex_lock(&mtx);
        status = SHUTING_DOWN;
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mtx);
        int ret = pthread_join(server_tid, NULL);
#endif
}

void Rpc::register_rpc_handler(int req_type,
                size_t(*handler_) (uint8_t* resp_buf,
                        uint8_t *resp_type,
                        const uint8_t* req_buf,
                        size_t req_len, void *arg),
                void *arg) {
        rpc_handler[req_type] = handler_;
        rpc_handler_arg[req_type] = arg;
}

RpcReq * Rpc::new_req(uint8_t req_type, int to_which_node, void* resp_buf,
                size_t max_resp_len) {
#ifdef RPC_DEBUG
        printf("%s\n",__PRETTY_FUNCTION__);
#endif

        int req_index = req_batch.num_reqs;

        RpcReq * req = &req_batch.reqs[req_index];
    
        req->resp_buf = (uint8_t*)resp_buf;
        req->max_resp_len = max_resp_len;
        req_batch.num_reqs++;
        int c_msg_index = -1;

        if (req_batch.c_msg_for_node[to_which_node] >= 0) {
                c_msg_index = req_batch.c_msg_for_node[to_which_node];
        }
        else {
                c_msg_index = req_batch.next_avaliable_c_msg_slot;
                req_batch.c_msg_for_node[to_which_node] = c_msg_index;
                req_batch.next_avaliable_c_msg_slot++;

                req_batch.c_msg[c_msg_index].node_id = to_which_node;
        }
        RpcCoalMsg * cmsg = &req_batch.c_msg[c_msg_index];
        RpcMsgHdr * msg_hdr = (RpcMsgHdr *) ((cmsg->req_buf).cur_ptr);
        msg_hdr->msg_type = req_type;
        msg_hdr->rpc_seq = rpc_seq++;
        cmsg->req_buf.cur_ptr += sizeof(RpcMsgHdr);
        req->req_buf = cmsg->req_buf.cur_ptr;
        req->msg_hdr = msg_hdr;
        req->cmsg_buf = &(cmsg->req_buf);
        (cmsg->num)++;

#ifdef RPC_DEBUG
        // printf("%s\n",__PRETTY_FUNCTION__);
        printf("\tNew request started:\n\treq type:\t%d\n\treq buff addr:\t%ld\n\tto node:\t%d\n\tresp buff addr:\t%ld\n\tmax resp len:\t%d\n",
                        (int)req_type, (long)req->req_buf,
                        (int)to_which_node, (long)resp_buf,
                        (int)max_resp_len);
#endif
        return req;
}


Buffer* Rpc::new_resp(int resp_to_whom, int num_reqs, uint32_t req_imm) {
/*
        RpcCoalMsg * c_msg = &resp_batch.c_msg[resp_batch.num_c_msg++];
        c_msg->node_id = resp_to_whom;
        c_msg->num = num_reqs;
        return &(c_msg->resp_buf);
*/
}


void Rpc::clear_req_batch() {
        req_batch.clear();
} 

// Because of the RDMA lib has not yet been finished,
// This function is currently used only to print reqs for debugging
void Rpc::send_reqs() {
#ifdef RPC_DEBUG
        printf("%s\n", __PRETTY_FUNCTION__);
        RpcReqBatch * batch = &req_batch;
        for (int i = 0; i < MAX_NODES; i++) {
                if (batch->c_msg_for_node[i] != -1) {
                        int index = batch->c_msg_for_node[i];
                        RpcCoalMsg * msg = &((batch->c_msg)[index]);
                        printf("Request for node %d\n", i);
                        printf("---------------MSG BUF PRETTY PRINT---------------\n");
                        printf("to node: \t%d\n", msg->node_id);
                        printf("num of req: \t%d\n", msg->num);
                        printf("req buf:\n");
                        uint8_t  * ptr = (msg->req_buf).head_ptr;
                        for (int j = 0; j < msg->num; j++) {
                                RpcMsgHdr* hdr = (RpcMsgHdr*) ptr;
                                printf("\tmsg type:\t%d\n", hdr->msg_type);
                                printf("\tmsg size:\t%d\n", hdr->size);
                                printf("\trpc seq:\t%d\n", hdr->rpc_seq);
                                ptr += sizeof(RpcMsgHdr);
                                DsReadReq * req = (DsReadReq *) ptr;
                                printf("\tds req type:\t%ld\n", (long)(req->req_type));
                                printf("\tds req address:\t%ld\n", (long)(req->address));
                                printf("\tds req length:\t%ld\n", (long)(req->length));
                                ptr += sizeof(DsReadReq);
                                printf("END REQ\n");
                        }
                        printf("-------------------MSG BUF END-------------------\n");
                }
        }
#ifdef LOCAL_SIM
        pthread_barrier_init(&barrier, NULL, 2);
        pthread_mutex_lock(&mtx);
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mtx);
        pthread_barrier_wait(&barrier);
        pthread_barrier_destroy(&barrier);
#endif

#endif
}

void Rpc::send_resp() {
#ifdef LOCAL_SIM
        pthread_mutex_lock(&recv_mtx);
        pthread_cond_signal(&recv_cond);
        pthread_mutex_unlock(&mtx);
#endif
}

void Rpc::recv_resp() {
/*
#ifdef LOCAL_SIM
        pthread_mutex_lock(&recv_mtx);
        pthread_cond_wait(&recv_cond, &recv_mtx);
        for (int i = 0; i < resp_batch.num_c_msg; i++) {
                RpcCoalMsg * c_msg = &resp_batch.c_msg[i];
                Buffer * buf = &c_msg->resp_buf;
                uint8_t* ptr = buf->cur_ptr;
                for (int j = 0; j < c_msg->num; j++) {
                        DsReadResp * resp_head = (DsReadResp *)ptr;
                        uint8_t* local_address = resp_head->local_address;
                        DsRespType type = resp_head->resp_type;
                        size_t length = resp_head->length;
                        if (type == READ_SUCCESS)
                                for (int k = 0; k < req_batch.num_reqs; k++) 
                                        if (req_batch.reqs[i].req_resp_buf == local_address) 
                                                req_batch.reqs[i].resp_type = READ_SUCCESS;
                        memcpy(local_address, ptr + sizeof(DsReadResp), length);
                        ptr += sizeof(DsReadResp) + length;
                }
        }
        pthread_mutex_unlock(&mtx);
#endif
*/
}