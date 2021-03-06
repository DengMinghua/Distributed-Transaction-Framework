#ifndef TX_DEF_H_
#define TX_DEF_H_

#define MAX_TX_RPC 256
#define MAX_BLOCKS_PER_REQUEST 256
#define MAX_ITEM_RESP_BUF 4096

#include <cstddef>
#include <stdint.h>
enum TxStatus {
    PROGRESSING,
    COMMITTED,
    ABORTING,
    ABORTED
};

enum TxRwMode {
    READ,
    UPDATE
};

/*enum TxRpcType {
    RPC_READ,
    RPC_READNLOCK,
    RPC_WRITE,
    RPC_COMMIT,
    RPC_UNLOCK
};*/

//typedef uint8_t* TxRwAddress;
//typedef size_t TxRwLength;

#endif
