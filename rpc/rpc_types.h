#ifndef RPC_TYPES_H_
#define RPC_TYPES_H_

#define RPC_TYPE_NUM 5

enum RPCType {
    RPC_READ,
    RPC_READNLOCK,
    RPC_WRITE,
    RPC_COMMIT,
    RPC_UNLOCK
};

#endif
