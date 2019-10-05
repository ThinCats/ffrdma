#include "mpi.hpp"
#include "stdio.h"

int main(int argc, char** argv)
{
    RDMA_Init(argc, argv);
    int local_rank = RDMA_Rank();
    printf("local_rank = %d", local_rank);
    RDMA_Finalize();
}