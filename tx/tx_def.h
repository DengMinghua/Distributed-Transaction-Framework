#ifndef TX_DEF_H_
#define TX_DEF_H_

#define MAX_TX_RPC 256

enum TxStatus {
    PROGRESSING,
    COMMITTED,
    ABORTED
}

enum TxRwMode {
    READ,
    UPDATE,
    INSERT,
    DELETE
}

enum TxRpcType {
    READ,
    READNLOCK,
    WRITE,
    COMMIT,
    UNLOCK
}

typedef void* TxRwAddress;
typedef size_t TxRwLength;

#endif
