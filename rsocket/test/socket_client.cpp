#include "../include/rdma_socket.h"
#include <stdio.h>
#include <stdlib.h>

#define MSG_LEN 32
#define MSG "A AMessage from client!@#$%^&*()"
#define MSG_COUNT 50

int main(int argc, char **argv) {
    //int msg_len,
    Socket *socket;
    char ch;

    socket = socket_(RDMA_PS_TCP);

    while(1){

    int res = connect_(&socket, argv[1], argv[2], 198);
    if(res == 0) break;
    close_(socket);
    socket= socket_(RDMA_PS_TCP);

    }
    int i;
    char msg[31];
    memcpy(msg,MSG,MSG_LEN);
    for(int i = 0; i < MSG_COUNT; i++){
        printf("Waiting for send command\n");
        scanf("%c",&ch);
        while(ch=='\n') scanf("%c",&ch);
        // send msg
        AMessage *msg;
        msg = AMessage_create((void *)MSG, sizeof(MSG), 0);
        if(send_(socket, msg))break;
        printf("%s\n", msg->buffer);
        printf("node_id, %d\n", msg->node_id);
        AMessage_destroy(msg);
    }

    close_(socket);
}
