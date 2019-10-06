#ifndef MPI_HPP
#define MPI_HPP
#include "malloc.h"
#include "fmpi.hpp"

#define R_TYPE_DOUBLE 0
#define R_TYPE_INT 1
#define R_TYPE_LONG_INT 2
#define R_TYPE_BYTE 3

#define R_OP_SUM 0
#define R_OP_MIN 1
#define R_OP_MAX 2

const int type_static[]={8,4,8,1};

int RDMA_Send(void *buf, int count, int type, int dest);
int RDMA_Recv(void *buf, int count, int type, int source);
int RDMA_Irecv(void *buf, int count, int type, int source);
// datatype  0 double 1 int 2 long int
// op 0 sum 1 min 2 max
int RDMA_Reduce(void *sendbuf, void *recvbuf, int count, int datatype, int op, int root);
int RDMA_Allreduce(void *sendbuf, void *recvbuf, int count, int datatype, int op);
int RDMA_Bcast(void *buf, int count_in_byte, int type ,int root);
int RDMA_GetOffsetRank(int offset, int is_right_side);
int RDMA_Allgather_exp(void *sendbuf, int sendcount, int sendtype, void *recvbuf, int recvcount, int recvtype);
int RDMA_Allgatherv_exp(void *sendbuf, int sendcount, int sendtype, void *recvbuf, int *recvcount, int *displs, int recvtype);
int RDMA_Allgather(void *sendbuf, int sendcount, int sendtype, void *recvbuf, int recvcount, int recvtype);
int RDMA_Gather(void *sendbuf, int sendcount, int sendtype, void *recvbuf, int recvcount, int recvtype, int root);
int RDMA_Scatter(void *sendbuf, int sendcount, int sendtype, void *recvbuf, int recvcount, int recvtype, int root);
int RDMA_Barrier();
int TestIrecv();
#endif
