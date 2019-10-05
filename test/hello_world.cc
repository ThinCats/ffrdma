#include "mpi.hpp"
#include "stdio.h"
#include "amessage.h"
#include <malloc.h>
#include "rdma_socket.h"

int main(int argc, char** argv)
{
    RDMA_Init(argc, argv);
    int local_rank = RDMA_Rank();
    printf("local_rank = %d\n", local_rank);
    int other_rank = local_rank ? 0 : 1;
    Socket *socket = RDMA_Socket(other_rank);
    printf("socket: %x\n", socket);
    if (local_rank == 0) {
        const char *msg = "Hello";
        RDMA_Send(msg, 6, other_rank, 0);
    }
    else
    {
        auto buf = (char *)malloc(10);
        RDMA_Recv(buf, 6, other_rank, 0);
        printf("local_rank = %d\n", local_rank);
        printf("%s\n", buf);
    }
    while(1)
        ;
    // RDMA_Finalize();
}