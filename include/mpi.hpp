#ifndef MPI_HPP
#define MPI_HPP
#include "fmpi.hpp"
    int RDMA_Send(const void *buf, int count, int dest,int rdma_group);
    int RDMA_Recv(void *buf, int count, int source, int rdma_group);
    int RDMA_Reduce(const void *sendbuf, void *recvbuf, int count, int datatype, int op, int root, int rdma_group);
    int RDMA_Allreduce(const void *sendbuf, void *recvbuf, int count, int datatype, int op, int rdma_group);
    int RDMA_MakeAll(void *buf, int count_in_byte, int root, int rdma_group);
    int RDMA_GetOffsetRank(int offset, int is_right_side, int rdma_group);
    int RDMA_ExchangeAll_exp(const void *sendbuf, int sendcount, void *recvbuf, int recvcount, int rdma_group);
    int RDMA_ExchangeAll(const void *sendbuf, int sendcount, void *recvbuf, int recvcount, int rdma_group);
    int RDMA_GetAll(const void *sendbuf, int sendcount, void *recvbuf, int recvcount, int root, int rdma_group);
    int RDMA_Scatter(const void *sendbuf, int sendcount, void *recvbuf, int recvcount, int root, int rdma_group);
#endif