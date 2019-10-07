#include "mpi.hpp"
#include "stdio.h"
#include "amessage.h"
#include "rdma_socket.h"

int main(int argc, char** argv)
{
    RDMA_Init(&argc, &argv);
    int local_rank = RDMA_Rank();
    printf("local_rank = %d\n", local_rank);


    int revct[]={1,2,1,3};
    int recdp[]={0,1,3,4};

    double msg_buf[3];
    for(int i=0;i<3;i++){
      msg_buf[i]=21348.3782*(local_rank*local_rank*local_rank+i+i*local_rank*(local_rank+3));
    }
    double rebuf[7];
    for(int i=0;i<7;i++){
      rebuf[i]=0;
    }

    printf("local_rank:%d   msg:%f %f %f\n",local_rank,msg_buf[0],msg_buf[1],msg_buf[2]);
    RDMA_Allgatherv_exp((void *)msg_buf,revct[local_rank],R_TYPE_DOUBLE,(void *)rebuf,revct,recdp,R_TYPE_DOUBLE);

    printf("recv:\n");
    for(int i=0;i<7;i++){
    printf("%f    ",rebuf[i]);
    }

    printf("down\n");

    RDMA_Finalize();
}
