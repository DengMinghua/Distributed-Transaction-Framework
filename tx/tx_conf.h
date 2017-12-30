#ifndef TX_CONF_H_
#define TX_CONF_H_

#include<assert.h>

#define TX_DEBUG_ASSERT 0
#define tx_assert(x) \
    do { if (TX_DEBUG_ASSERT) assert(x); } while (0)

#define TX_MAX_BACKUPS 2

#define TX_MAX_READ_SET 256
#define TX_MAX_WRITE_SET 256

#endif

