#ifndef FMPI_HPP
#define FMPI_HPP

const static int RDMA_SUCCESS = 0;
const static int RDMA_ERR_COMM = 1;
const static int RDMA_INTERN = 2;

struct Socket_;
typedef Socket_ Socket;
typedef int RDMA_Comm;
int RDMA_Init(int *argc, char ***argv);
int RDMA_Finalize();
int RDMA_Rank(RDMA_Comm comm = 0);
int RDMA_Size(RDMA_Comm comm = 0);
double RDMA_Wtime();
Socket *const RDMA_Socket(int toRank, RDMA_Comm comm = 0);
Socket *const RDMA_Reconnect(int toRank, RDMA_Comm comm = 0);
int RDMA_Comm_split(RDMA_Comm comm, int color, int key, RDMA_Comm *newcomm);
// !! Warning, not implemented
int RDMA_Comm_free(RDMA_Comm *comm);
// !! Warning, not implemented
int RDMA_Abort(RDMA_Comm comm, int errorcode);
#endif // FMPI_HPP