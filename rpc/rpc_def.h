#ifndef RPC_DEF_H_
#define RPC_DEF_H_

#define MAX_NODES 8

#define RPC_MAX_QPS 3
#define RPC_MAX_POSTLIST 64

#define MAX_RPC_BATCH_SIZE 64
#define MAX_COAL_SIZE 64

typedef uint8_t RespBuf;
typedef uint8_t ReqBuf;
typedef uint8_t MsgType;

struct Buffer {
    uint8_t * head_ptr;
    size_t buf_size;

    uint8_t * cur_ptr;

    inline void alloc(size_t len) {
        head_ptr = (uint8_t *) memalign(8, len);
        assert(head_ptr != NULL);
        buf_size = len;
        cur_ptr = buf;
    }

    inline size_t length() {
        return (size_t) (cur_ptr-head_ptr);
    }

    inline void reset() {
        cur_ptr = head_ptr;
    }

    inline size_t ava_bytes() {
        return (buf_size - length());
    }
}


struct RpcReq {
    ReqBuf * req_buf;
    size_t req_len;

    RespBuf * resp_buf;
    size_t max_resp_len;
    size_t resp_len;

    Buffer * cmsg_buf;
    RpcMsgHdr * cmsg_hdr;

    MsgType resp_type;

    void freeze(size_t req_len_) {
        cmsg_hdr->size = req_len_;

        req_len = req_len_;
        cmsg_buf->cur_ptr += req_len_;
    }
}

struct RpcCoalMsg {
    int node_id;
    int num;
    
    Buffer req_buf;
    Buffer resp_buf;
}

struct RpcMsgHdr {
    MsgType msg_type;
    uint16_t size;
}

struct RpcReqBatch {
    RpcReq reqs[MAX_RPC_BATCH_SIZE];

    int num_reqs;
    int num_reqs_completed;

    RpcCoalMsg c_msg[MAX_COAL_SIZE];
    int index_c_msg_for_node[MAX_NODES];
    int next_avaliable_c_msg_slot;
    
    inline void clear() {
        assert(num_reqs_completed == num_reqs);
        for (int i = 0; i < num_nodes; i++) {
            c_msg[i].node_id = -1;
            c_msg[i].num = 0;
            c_msg[i].req_buf.reset();
            c_msg[i].resp_buf.reset();
        }
        next_avaliable_c_msg_slot = 0;
    }

  
}


#endif