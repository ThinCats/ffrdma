#include "mpi.hpp"
#include "stdio.h"
#include "amessage.h"
#include "rdma_socket.h"

int main(int argc, char** argv)
{
    RDMA_Init(argc, argv);
    int local_rank = RDMA_Rank();
    printf("local_rank = %d\n", local_rank);

    double msg_buf[]={1.1,2.2,3.3,4.4};

    if(local_rank == 0){
    msg_buf[0]=111.11;
    msg_buf[1]=222.22;
    msg_buf[2]=333.33;
    msg_buf[3]=444.44;
    }

    printf("local_rank:%d   msg:%f %f %f %f\n",local_rank,msg_buf[0],msg_buf[1],msg_buf[2],msg_buf[3]);
    RDMA_Scatter((void *)msg_buf,8,(void *)&msg_buf[local_rank],8,0,0);
    printf("local_rank:%d   msg:%f %f %f %f\n",local_rank,msg_buf[0],msg_buf[1],msg_buf[2],msg_buf[3]);

    RDMA_Finalize();
}
