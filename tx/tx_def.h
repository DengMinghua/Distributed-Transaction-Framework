#ifndef TX_DEF_H_
#define TX_DEF_H_

#define MAX_TX_RPC 256

#include <cstddef>

enum TxStatus {
    PROGRESSING,
    COMMITTED,
    ABORTING,
    ABORTED
};

enum TxRwMode {
    READ,
    UPDATE,
    INSERT,
    DELETE
};

enum TxRpcType {
    RPC_READ,
    RPC_READNLOCK,
    RPC_WRITE,
    RPC_COMMIT,
    RPC_UNLOCK
};

typedef void* TxRwAddress;
typedef size_t TxRwLength;

#endif
