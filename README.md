# Distributed-Transaction-Framework
An under-constructed distributed transaction framework based on two-sided RDMA.

## Finished
* Basic RPC class (start new request and response/ coalesce request to message batch/ start new commit request)
* Basic transaction management class (start new transaction/ add to read and write set/ start new RPC request for reading data/ read validation/ transaction abort)
* Basic data store class (forge new read/write/lock data store request)
* Basic mapping class (static mapping)
* Simple Debug mode for major functions

## Ongoing
* Callback function on server side (forge response for read request/ handle lock request(data partition, locked list)/ handle data commit(logger/ call rmsync()/ response Ack).

## Todo
* RPC network part (waiting for the two sided RDMA library)
* Functions to allocate temp buffer for messsage passing.
* Simulator to test basic functionality in local machine.
* Add more local tests

