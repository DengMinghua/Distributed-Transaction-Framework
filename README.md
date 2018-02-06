# Distributed-Transaction-Framework
An under-constructed distributed transaction framework based on two-sided RDMA.

## Finished
* Basic RPC module (start new request and response/ coalesce request to message batch/ register message handler)
* Basic transaction management module (start new transaction/ add item to read or write set/ start new RPC request for reading data/ read validation/ transaction abort)
* Basic mapping class (shared memory mapping/ memory block version/ memory block locking)
* Several test callback function on server side (for data read and lock&read request)
* Debug mode for major modules
* Local simulation mode that can go through the execute/ validation phase

## Ongoing
* Modify commit part to fit with two phase commit model

## Todo
* Integrate RDMA lib into RPC module 
