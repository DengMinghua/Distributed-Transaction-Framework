# Distributed-Transaction-Framework
An under-constructed distributed transaction framework based on two-sided RDMA.

## Finished
* Basic RPC module (start new request and response/ coalesce request to message batch/ register message handler)
* Basic transaction management module (start new transaction/ add item to read or write set/ start new RPC request for reading data/ read validation/ transaction abort)
* Basic mapping class (shared memory mapping/ memory block version/ memory block locking)
* Several test callback function on server side (for data read and lock&read request)
* Debug mode for major modules
* Local simulation mode that can go through the execute/ validation phase
* Switch from shared memory based to object based

## Ongoing
* Modify commit part to fit with two phase commit model

## Todo
* See whether single-thread sequential read is better than multi threads async read
* Integrate RDMA lib into RPC module (multi client threads vs single client thread for sending / one thread server for computing and response)  


