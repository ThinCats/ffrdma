#include "mpi.hpp"
#include "stdio.h"
#include "amessage.h"
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
        AMessage *amsg = AMessage_create((void *)msg, 6, 0);
        if (send_(socket, amsg)) {
            printf("Error to send");
        }
        else {
            printf("nodeid: %d\n", amsg->node_id);
        }
        Socket *newSocket = RDMA_Reconnect(other_rank);
        if (send_(newSocket, amsg)) {
            printf("Error to send with new socket");
        }
        else {
            printf("nodeid: %d\n", amsg->node_id);
        }
        AMessage_destroy(amsg);
    } else {
        AMessage *recv;
        while(true) {
            recv = recv_(socket);
            if (recv == nullptr) {
                printf("Stop\n");
                break;
            }
            printf("%s, nodeId: %d\n", recv->buffer, recv->node_id);
            socket = RDMA_Reconnect(other_rank);
            AMessage_destroy(recv);
        }
    }

    printf("Reach end\n");
    if (local_rank == 0) {
        while(1);
    }
    RDMA_Finalize();
}