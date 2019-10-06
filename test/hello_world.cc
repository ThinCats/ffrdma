#include "mpi.hpp"
#include "stdio.h"
#include "amessage.h"
#include <malloc.h>
#include "rdma_socket.h"
#include <thread>

int main(int argc, char** argv)
{
    RDMA_Init(argc, argv);
    int local_rank = RDMA_Rank();
    printf("local_rank = %d\n", local_rank);
    int other_rank = local_rank ? 0 : 1;
    Socket *socket = RDMA_Socket(other_rank);
    printf("socket: %x\n", socket);
    if (local_rank == 0) {
        char *msg1 = "Hello";
        RDMA_Send(msg1, 6, other_rank, 0);
        char *msg2 = "World";
        RDMA_Send(msg2, 6, other_rank, 0);
        char *msg3 = "!";
        RDMA_Send(msg3, 2, other_rank, 0);
        auto recvbuf = (int *)malloc(100);
        auto num = (int *)malloc(100);
        for (int i = 0; i < 5;i ++)
            num[i] = i + 1;
        RDMA_Reduce(num,recvbuf, 20, 1, 0, 0, 0);
        for (int i = 0; i < 5;i++){
            printf("%d ", recvbuf[i]);
        }
        for (int i = 0; i < 5;i ++)
            num[i] = i + 1;
        RDMA_Reduce(num,recvbuf, 20, 1, 1, 0, 0);
        for (int i = 0; i < 5;i++){
            printf("%d ", recvbuf[i]);
        }
        for (int i = 0; i < 5;i ++)
            num[i] = i + 1;
        RDMA_Reduce(num,recvbuf, 20, 1, 2, 0, 0);
        for (int i = 0; i < 5;i++){
            printf("%d ", recvbuf[i]);
        }
        
        auto recvbuf_double = (double *)malloc(100);
        auto num_double = (double *)malloc(100);
        for (int i = 0; i < 5;i ++)
            num_double[i] = i + 1;
        RDMA_Reduce(num_double,recvbuf_double, 40, 0, 0, 0, 0);
        for (int i = 0; i < 5;i++){
            printf("%.6lf ", recvbuf_double[i]);
        }
        for (int i = 0; i < 5;i ++)
            num_double[i] = i + 1;
        RDMA_Reduce(num_double,recvbuf_double, 40, 0 ,1, 0, 0);
        for (int i = 0; i < 5;i++){
            printf("%.6lf ", recvbuf_double[i]);
        }
        for (int i = 0; i < 5;i ++)
            num_double[i] = i + 1;
        RDMA_Reduce(num_double,recvbuf_double, 40, 0, 2, 0, 0);
        for (int i = 0; i < 5;i++){
            printf("%.6lf ", recvbuf_double[i]);
        }
        printf("\n");
        printf("begin barrier\n");
        RDMA_Barrier();
        printf("end barrier\n");
        TestIrecv();
        free(recvbuf);
        free(recvbuf_double);
        free(num_double);
        free(num);
    }
    else
    {
        auto buf = (char *)malloc(100);
        RDMA_Recv(buf, 6, other_rank, 0);
        printf("local_rank = %d\n", local_rank);
        printf("%s\n", buf);
        RDMA_Recv(buf, 6, other_rank, 0);
        printf("%s\n", buf);
        RDMA_Recv(buf, 2, other_rank, 0);
        printf("%s\n", buf);
        free(buf);
        auto recvbuf = (int *)malloc(100);
        auto num = (int *)malloc(100);
        for (int i = 0; i < 5;i ++)
            num[i] = 5 - i;
        RDMA_Reduce(num,recvbuf, 20, 1, 0, 0, 0);
        for (int i = 0; i < 5;i ++)
            num[i] = 5 - i;
        RDMA_Reduce(num,recvbuf, 20, 1, 1, 0, 0);
        for (int i = 0; i < 5;i ++)
            num[i] = 5 - i;
        RDMA_Reduce(num,recvbuf, 20, 1, 2, 0, 0);
        
        auto recvbuf_double = (double *)malloc(100);
        auto num_double = (double *)malloc(100);
        for (int i = 0; i < 5;i ++)
            num_double[i] =  5 - i;
        RDMA_Reduce(num_double,recvbuf_double, 40, 0, 0, 0, 0);
        for (int i = 0; i < 5;i ++)
            num_double[i] =  5 - i;
        RDMA_Reduce(num_double,recvbuf_double, 40, 0 ,1, 0, 0);
        for (int i = 0; i < 5;i ++)
            num_double[i] =  5 - i;
        RDMA_Reduce(num_double,recvbuf_double, 40, 0, 2, 0, 0);
        printf("\n");
        std::this_thread::sleep_for (std::chrono::seconds(5));
        RDMA_Barrier();
        TestIrecv();
        free(recvbuf);
        free(recvbuf_double);
        free(num_double);
        free(num);
    }
    while(1)
        ;
    // RDMA_Finalize();
}