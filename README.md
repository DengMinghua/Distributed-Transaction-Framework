# Distributed-Transaction-Framework
An under-constructed distributed transaction framework based on two-sided RDMA.

## Finished
* Basic RPC class (start new request and response/ coalesce request to message batch/ start new commit request)
* Basic transaction management class (start new transaction/ add to read and write set/ start new RPC request for reading data/ read validation/ transaction abort)
* Basic data store class (forge new read/write/lock data store request)
* Basic mapping class (static mapping)
* Simple Debug mode for major functions

## Todo
* Add a Makefile and some simple tests
* Functions to allocate temp buffer for messsage passing.
* Callback function on server side (forge response for read request/ handle lock request(data partition, locked list)/ handle data commit(logger/ call rmsync()/ response Ack).
* Simulator to test basic functionality in local machine.
* RPC network part (waiting for the two sided RDMA library)
