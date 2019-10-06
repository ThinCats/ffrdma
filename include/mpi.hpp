#ifndef MPI_HPP
#define MPI_HPP
#include "fmpi.hpp"
    int RDMA_Send(void *buf, int count, int dest,int rdma_group);
    int RDMA_Recv(void *buf, int count, int source, int rdma_group);
    // datatype  0 double 1 int 2 long int
    // op 0 sum 1 min 2 max
    int RDMA_Reduce(void *sendbuf, void *recvbuf, int count, int datatype, int op, int root, int rdma_group);
    int RDMA_Allreduce(void *sendbuf, void *recvbuf, int count, int datatype, int op, int rdma_group);
    int RDMA_MakeAll(void *buf, int count_in_byte, int root, int rdma_group);
    int RDMA_GetOffsetRank(int offset, int is_right_side, int rdma_group);
    int RDMA_ExchangeAll_exp(void *sendbuf, int sendcount, void *recvbuf, int recvcount, int rdma_group);
    int RDMA_ExchangeAll(void *sendbuf, int sendcount, void *recvbuf, int recvcount, int rdma_group);
    int RDMA_GetAll(void *sendbuf, int sendcount, void *recvbuf, int recvcount, int root, int rdma_group);
    int RDMA_Scatter(void *sendbuf, int sendcount, void *recvbuf, int recvcount, int root, int rdma_group);
    int RDMA_Barrier();
    int TestIrecv();
#endif