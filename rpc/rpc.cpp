#include "rpc.h"

void* Rpc::local_sim_rpc_listener() {

#ifdef RPC_DEBUG
	printf("%s\n", __PRETTY_FUNCTION__);
	printf("Local Simlulation Server UP!\n");
	printf("[Server] Server is listening\n");
#endif

	pthread_barrier_wait(&barrier);

	while (status == ONLINE) {
		pthread_mutex_lock(&mtx);

		while (!send_signal) {
			pthread_cond_wait(&cond, &mtx);
			if (status == SHUTING_DOWN) {
				break;
			}

			RpcReqBatch * batch = &req_batch;

			for (int i = 0; i < MAX_NODES; i++) {
				if (batch->c_msg_for_node[i] != -1) {
#ifdef RPC_DEBUG
					printf("[Server] Recieved Message for node %d\n", i);
#endif
					int index = batch->c_msg_for_node[i];
					RpcCoalMsg * msg = &((batch->c_msg)[index]);

					uint8_t * recv_msg_ptr = (msg->req_buf).head_ptr;
					uint8_t * recv_msg_end_ptr = (msg->req_buf).cur_ptr;

					Buffer* resp_buffer = new_resp(i);

					while (recv_msg_ptr < recv_msg_end_ptr) {

						RpcMsgHdr* hdr = (RpcMsgHdr*) recv_msg_ptr;
						memcpy(resp_buffer->cur_ptr, hdr, sizeof(RpcMsgHdr));                                    

						size_t req_buf_size = hdr->size;
						uint8_t req_type = hdr->msg_type;

						uint8_t* resp_type = &(((RpcMsgHdr*)(resp_buffer->cur_ptr))->msg_type);
						uint16_t* resp_size = &(((RpcMsgHdr*)(resp_buffer->cur_ptr))->size);

						resp_buffer->cur_ptr += sizeof(RpcMsgHdr);
						recv_msg_ptr += sizeof(RpcMsgHdr);

						size_t buf_size = rpc_handler[req_type]((uint8_t*) resp_buffer->cur_ptr, resp_type, 
								(uint8_t*)recv_msg_ptr, sizeof(DsReadReq), 
								rpc_handler_arg[req_type]);

						resp_buffer->cur_ptr += buf_size;
						recv_msg_ptr += req_buf_size;

						(*resp_size) =  buf_size;
					}
				}
			}
			send_resp();
		}

		send_signal = false;
		pthread_mutex_unlock(&mtx);

	}

#ifdef RPC_DEBUG
	printf("[Server] Server is offline\n");
#endif

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
	send_signal = true;
#ifdef RPC_DEBUG
	printf("[Server] Server is shuting down\n");
#endif
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
	req->rpc_seq = msg_hdr->rpc_seq;
	
	(cmsg->num)++;

#ifdef RPC_DEBUG
	printf("\tNew request started:\n\treq type:\t%d\n\treq buff addr:\t%ld\n\tto node:\t%d\n\tresp buff addr:\t%ld\n\tmax resp len:\t%d\n",
			(int)req_type, (long)req->req_buf,
			(int)to_which_node, (long)resp_buf,
			(int)max_resp_len);
#endif

	return req;
}


Buffer* Rpc::new_resp(int resp_to_whom) {
	RpcCoalMsg * c_msg = &resp_batch.c_msg[resp_batch.num_c_msg++];
	c_msg->node_id = resp_to_whom;
	c_msg->num = 0;
	return &(c_msg->resp_buf);
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
	printf("[Client] request sent\n");
	pthread_mutex_lock(&mtx);
	send_signal = true;
	pthread_cond_signal(&cond);
	pthread_mutex_unlock(&mtx);
#endif

#endif
}


void Rpc::send_resp() {
#ifdef LOCAL_SIM
#ifdef RPC_DEBUG
	printf("[Server] response sent\n");
#endif
	pthread_mutex_lock(&recv_mtx);
	recv_signal = true;
	pthread_cond_signal(&recv_cond);
	pthread_mutex_unlock(&recv_mtx);
#endif
}


void Rpc::recv_resp() {
#ifdef LOCAL_SIM
#ifdef RPC_DEBUG
	printf("[Client] waiting for response\n");
#endif
	pthread_mutex_lock(&recv_mtx);
	while (!recv_signal) {
		pthread_cond_wait(&recv_cond, &recv_mtx);
#ifdef RPC_DEBUG
		printf("[Client] response recieved\n");
#endif
		for (int i = 0; i < resp_batch.num_c_msg; i++) {
			
			RpcCoalMsg * c_resp = &resp_batch.c_msg[i];
			Buffer * resp_buf = &(c_resp->resp_buf);
			
			uint8_t * head_ptr = resp_buf->head_ptr;
			uint8_t * cur_ptr = resp_buf->cur_ptr;
			
			while (head_ptr < cur_ptr) {
				RpcMsgHdr * hdr = (RpcMsgHdr *) head_ptr;
				int seq = hdr->rpc_seq;
				int type = hdr->msg_type;
				int size = hdr->size;
				head_ptr += sizeof(RpcMsgHdr);

				for (int j = 0; j < req_batch.num_reqs; j++) {
					if (req_batch.reqs[j].rpc_seq == seq) {
						printf("[Client] response matched\n");
						req_batch.reqs[j].resp_type = type;
						req_batch.reqs[j].resp_buf = head_ptr;
					}
				}
				head_ptr += size;
			}
		}
	}
	recv_signal = false;
	pthread_mutex_unlock(&recv_mtx);
#endif

}

