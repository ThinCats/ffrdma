#ifndef MPI_HPP
#define MPI_HPP
#include "fmpi.hpp"
#include "malloc.h"

#define R_TYPE_DOUBLE 0
#define R_TYPE_INT 1
#define R_TYPE_LONG_INT 2
#define R_TYPE_BYTE 3

#define R_OP_SUM 0
#define R_OP_MIN 1
#define R_OP_MAX 2

const int type_static[] = {8, 4, 8, 1};

int RDMA_Send(void *buf, int count, int type, int dest, RDMA_Comm comm = 0);
int RDMA_Recv(void *buf, int count, int type, int source, RDMA_Comm comm = 0);
int RDMA_Irecv(void *buf, int count, int type, int source, RDMA_Comm comm = 0);
// datatype  0 double 1 int 2 long int
// op 0 sum 1 min 2 max
int RDMA_Reduce(void *sendbuf, void *recvbuf, int count, int datatype, int op,
                int root, RDMA_Comm comm = 0);
int RDMA_Allreduce(void *sendbuf, void *recvbuf, int count, int datatype,
                   int op, RDMA_Comm comm = 0);
int RDMA_Bcast(void *buf, int count_in_byte, int type, int root,
               RDMA_Comm comm = 0);
int RDMA_GetOffsetRank(int offset, int is_right_side, RDMA_Comm comm = 0);
int RDMA_Allgather_exp(void *sendbuf, int sendcount, int sendtype,
                       void *recvbuf, int recvcount, int recvtype,
                       RDMA_Comm comm = 0);
int RDMA_Allgatherv_exp(void *sendbuf, int sendcount, int sendtype,
                        void *recvbuf, int *recvcount, int *displs,
                        int recvtype, RDMA_Comm comm = 0);
int RDMA_Allgather(void *sendbuf, int sendcount, int sendtype, void *recvbuf,
                   int recvcount, int recvtype, RDMA_Comm comm = 0);
int RDMA_Gather(void *sendbuf, int sendcount, int sendtype, void *recvbuf,
                int recvcount, int recvtype, int root, RDMA_Comm comm = 0);
int RDMA_Scatter(void *sendbuf, int sendcount, int sendtype, void *recvbuf,
                 int recvcount, int recvtype, int root, RDMA_Comm comm = 0);
int RDMA_Barrier(RDMA_Comm comm = 0);
#endif
