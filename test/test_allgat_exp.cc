#include "mpi.hpp"
#include "stdio.h"
#include "amessage.h"
#include "rdma_socket.h"

int main(int argc, char** argv)
{
    RDMA_Init(&argc, &argv);
    int local_rank = RDMA_Rank();
    printf("local_rank = %d\n", local_rank);

    double msg_buf[]={1.1,2.2,3.3,4.4};

    msg_buf[local_rank]=234.567*local_rank;

    printf("local_rank:%d   msg:%f %f %f %f\n",local_rank,msg_buf[0],msg_buf[1],msg_buf[2],msg_buf[3]);
    RDMA_Allgather_exp((void *)&msg_buf[local_rank],1,R_TYPE_DOUBLE,(void *)msg_buf,1,R_TYPE_DOUBLE);
    printf("local_rank:%d   msg:%f %f %f %f\n",local_rank,msg_buf[0],msg_buf[1],msg_buf[2],msg_buf[3]);

    RDMA_Finalize();
}
