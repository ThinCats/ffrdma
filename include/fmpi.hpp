#ifndef FMPI_HPP
#define FMPI_HPP

struct Socket_;
typedef Socket_ Socket;
int RDMA_Init(int argc, char **argv);
int RDMA_Finalize();
int RDMA_Rank();
int RDMA_Size();
Socket *const RDMA_Socket(int toRank);
Socket *const RDMA_Reconnect(int toRank);

#endif // FMPI_HPP