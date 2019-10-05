#include "mpi.hpp"
#include "fmpi.hpp"
#include "receiver.h"
#include "rdma_socket.h"
#include <string.h>
#include <algorithm>

using namespace std;

int RDMA_MakeAll(void *buf, int count_in_byte, int root, int rdma_group)
{
    int local_rank = RDMA_Rank();
    int whole_ranks = RDMA_Size();
    int res =-1;
 
    if(buf == NULL) return 1;
    if(count_in_byte <= 0) return 2;
    if(root >= whole_ranks || root < 0) return 3;


    if (local_rank == root)
    {
        AMessage *msg = (AMessage *)1;
        msg = AMessage_create((void *)buf, count_in_byte, 0);
        for (int i = 0; i < whole_ranks; i++)
        {
            if (i == local_rank) continue;
            res = send_(RDMA_Socket(i), msg);
            if(res != 0) {AMessage_destroy(msg);return res;}
        }
        AMessage_destroy(msg);
    }
    else
    {
        auto msg = recv_(RDMA_Socket(root));
        if (msg == NULL)
            return 4;
        if (msg->length == count_in_byte && msg->node_id == root)
        {
            memcpy(buf, msg->buffer, count_in_byte);
            AMessage_destroy(msg);
        }else{
            AMessage_destroy(msg);
            return 5;
        }
    }
    return 0;
}

int RDMA_GetOffsetRank(int offset, int is_right_side, int rdma_group)
{
    int local_rank = RDMA_Rank();
    int whole_ranks = RDMA_Size();
    int offset_rank;

    if (is_right_side)
    {
        offset_rank = local_rank + offset;
        if (offset_rank > whole_ranks - 1)
        {
            offset_rank = offset_rank - whole_ranks;
        }
    }
    else
    {
        offset_rank = local_rank - offset;
        if (offset_rank < 0)
        {
            offset_rank = offset_rank + whole_ranks;
        }
    }

    return offset_rank;
}

int RDMA_ExchangeAll_exp(const void *sendbuf, int sendcount, void *recvbuf,
                         int recvcount, int rdma_group)
{
    int local_rank = RDMA_Rank();
    int whole_ranks = RDMA_Size();
    int res =-1;

    if(sendbuf == NULL || recvbuf == NULL) return 1;
    if(sendcount <= 0 || recvcount <= 0) return 2;

    unsigned char *buffer =
        (unsigned char *)malloc(sizeof(unsigned char) * sendcount);

    auto send_msg = AMessage_create((void *)sendbuf, sendcount, 0);

    int first_send_rank = RDMA_GetOffsetRank(1, 1, rdma_group);
    res = send_(RDMA_Socket(first_send_rank), send_msg);
    if(res != 0) {AMessage_destroy(send_msg);return res;}

    memcpy((unsigned char *)recvbuf + local_rank * recvcount, sendbuf, sendcount);
    for (int i = 2; i < whole_ranks; i++)
    {
        int send_rank = RDMA_GetOffsetRank(i, 1, rdma_group);
        int recv_rank = RDMA_GetOffsetRank(i - 1, 0, rdma_group);

        res = send_(RDMA_Socket(send_rank), send_msg);
        if(res != 0) {AMessage_destroy(send_msg);return res;}

        auto recv_msg = recv_(RDMA_Socket(recv_rank));
        if(recv_msg == NULL) {
            AMessage_destroy(send_msg);
            return 4;
        }
        memcpy((unsigned char *)recvbuf + recv_rank * recvcount, sendbuf,
               sendcount);
        AMessage_destroy(recv_msg);
    }
    AMessage_destroy(send_msg);

    int last_recv_rank = RDMA_GetOffsetRank(whole_ranks - 1, 0, rdma_group);
    auto recv_msg = recv_(RDMA_Socket(last_recv_rank));
    if(recv_msg == NULL){
        return 4;
    }
    memcpy((unsigned char *)recvbuf + last_recv_rank * recvcount, recv_msg->buffer, sendcount);
    AMessage_destroy(recv_msg);

    return 0;
}

int RDMA_ExchangeAll(const void *sendbuf, int sendcount, void *recvbuf,
                     int recvcount, int rdma_group)
{
    int local_rank = RDMA_Rank();
    int whole_ranks = RDMA_Size();
    int res =-1;

    if(sendbuf == NULL || recvbuf == NULL) return 1;
    if(sendcount <= 0 || recvcount <= 0) return 2;

    unsigned char *buffer =
        (unsigned char *)malloc(sizeof(unsigned char) * sendcount);
    for (int i = 0; i < whole_ranks; i++)
    {
        if (local_rank == i)
        {
            res = RDMA_MakeAll((void *)sendbuf, sendcount, i, rdma_group);
            if(res != 0) return res;
            
            memcpy(((unsigned char *)recvbuf) + sendcount * i, buffer, sendcount);
        }
        else
        {
            res = RDMA_MakeAll((void *)((unsigned char *)recvbuf + recvcount * i),
                         recvcount, i, rdma_group);
            if(res != 0) return res;
        }
    }

    return 0;
}

int RDMA_GetAll(const void *sendbuf, int sendcount, void *recvbuf,
                int recvcount, int root, int rdma_group)
{
    int local_rank = RDMA_Rank();
    int whole_ranks = RDMA_Size();
    int res =-1;

    if(sendbuf == NULL || recvbuf == NULL) return 1;
    if(sendcount <= 0 || recvcount <= 0) return 2;
    if(root >= whole_ranks || root < 0) return 3;

    if (local_rank == root)
    {
        for (int i = 0; i < whole_ranks; i++)
        {
            if (local_rank == i)
            {
                memcpy(((unsigned char *)recvbuf) + sendcount * i, sendbuf, sendcount);
            }
            else
            {
                AMessage *msg = (AMessage *)1;
                msg = recv_(RDMA_Socket(i));
                if (msg == NULL)
                    return 4;
                if (msg->length == recvcount && msg->node_id == i)
                {
                    memcpy(((unsigned char *)recvbuf) + recvcount * i, msg->buffer,
                            recvcount);
                    AMessage_destroy(msg);
                }else{
                    AMessage_destroy(msg);
                    return 5;
                }
            }
        }
    }
    else
    {
        AMessage *msg;
        msg = AMessage_create((void *)sendbuf, sendcount, 0);
        res = send_(RDMA_Socket(root), msg);
        AMessage_destroy(msg);
        if(res != 0) return res;
    }

    return 0;
}

int RDMA_Scatter(const void *sendbuf, int sendcount, void *recvbuf,
                 int recvcount, int root, int rdma_group)
{
    int local_rank = RDMA_Rank();
    int whole_ranks = RDMA_Size();
    int res =-1;

    if(sendbuf == NULL || recvbuf == NULL) return 1;
    if(sendcount <= 0 || recvcount <= 0) return 2;
    if(root >= whole_ranks || root < 0) return 3;

    if (local_rank == root)
    {
        for (int i = 0; i < RDMA_Size(); i++)
        {
            if (local_rank == i)
            {
                memcpy(recvbuf, ((unsigned char *)sendbuf) + sendcount * i, sendcount);
            }
            else
            {
                AMessage *msg;
                msg = AMessage_create(
                    (void *)((unsigned char *)sendbuf + sendcount * i), sendcount, 0);
                res = send_(RDMA_Socket(i), msg);
                if(res != 0){AMessage_destroy(msg);return res};
                AMessage_destroy(msg);
            }
        }
    }
    else
    {
        AMessage *msg = (AMessage *)1;
        msg = recv_(RDMA_Socket(root));
        if (msg == NULL)
            return -1;
        if (msg->length == recvcount && msg->node_id == root)
        {
            memcpy(recvbuf, msg->buffer, recvcount);
            AMessage_destroy(msg);
        }else{
            AMessage_destroy(msg);
            return 5;
        }
    }

    return 0;
}

int RDMA_Send(void *buf, int count, int dest, int rdma_group)
{
    int local_rank = RDMA_Rank();
    int rc = 1;
    if (count < 0)
    {
        rc = 0; // count error.
    }
    auto msg = AMessage_create(buf, count, 0);
    if (dest == local_rank)
    {
        return -1;
    }
    Socket *socket = RDMA_Socket(dest);
    send_(socket, msg);
    AMessage_destroy(msg);
}

int RDMA_Recv(void *buf, int count, int source, int rdma_group)
{
    int local_rank = RDMA_Rank();
    int rc = 1;
    if (local_rank == source)
    {
        return -1;
    }
    Socket *listen = RDMA_Socket(source);
    if (count < 0)
    {
        rc = 0;
        return rc;
    }
    AMessage *buffer = (AMessage *)1;
    buffer = recv_(listen);
    if (buffer->length == count && buffer->node_id == source)
    {
        memcpy(buf, buffer->buffer, count);
    }
    else
    {
        buf = nullptr;
    }
    AMessage_destroy(buffer);
    return rc;
}

// datatype  0 double 1 int 2 long int
// op 0 sum 1 min 2 max

int RDMA_Reduce(void *sendbuf, void *recvbuf, int count,
                int datatype, int op, int root, int rdma_group)
{
    int local_rank = RDMA_Rank();
    int whole_rank = RDMA_Size();
    if (local_rank == root)
    {
        memcpy(recvbuf, sendbuf, count);
        AMessage *buffer = (AMessage *)1;
        for (int i = 0; i < whole_rank; i++)
        {
            if (i == root)
                continue;
            Socket *listen = RDMA_Socket(i);
            auto msg = recv_(listen);
            if (datatype == 0)
            {
                double *resbuf = (double *)recvbuf;
                double *msgbuffer = (double *)msg->buffer;
                if (op == 0)
                {
                    for (int j = 0; j < count/8; j++)
                    {
                        resbuf[j] = resbuf[j] + msgbuffer[j];
                    }
                }
                else if (op == 1)
                {
                    for (int j = 0; j < count/8; j++)
                    {
                        resbuf[j] = min(resbuf[j], msgbuffer[j]);
                    }
                }
                else
                {
                    for (int j = 0; j < count/8; j++)
                    {
                        resbuf[j] = max(resbuf[j], msgbuffer[j]);
                    }
                }
            }
            else if (datatype == 1)
            {
                int *resbuf = (int*)recvbuf;
                int *msgbuffer = (int *)msg->buffer;
                if (op == 0)
                {
                    for (int j = 0; j < count / 4; j++)
                    {
                        resbuf[j] = resbuf[j] + msgbuffer[j];
                    }
                }
                else if (op == 1)
                {
                    for (int j = 0; j < count / 4; j++)
                    {
                        resbuf[j] = min(resbuf[j], msgbuffer[j]);
                    }
                }
                else
                {
                    for (int j = 0; j < count / 4; j++)
                    {
                        resbuf[j] = max(resbuf[j], msgbuffer[j]);
                    }
                }
            }
            else if (datatype == 2)
            {
                long long *resbuf = (long long *)recvbuf;
                long long *msgbuffer = (long long *)msg->buffer;
                if (op == 0)
                {
                    for (int j = 0; j < count/8; j++)
                    {
                        resbuf[j] = resbuf[j] + msgbuffer[j];
                    }
                }
                else if (op == 1)
                {
                    for (int j = 0; j < count/8; j++)
                    {
                        resbuf[j] = min(resbuf[j], msgbuffer[j]);
                    }
                }
                else
                {
                    for (int j = 0; j < count/8; j++)
                    {
                        resbuf[j] = max(resbuf[j], msgbuffer[j]);
                    }
                }
            }
        }
        AMessage_destroy(buffer);
    }
    else
    {
        auto msg = AMessage_create(sendbuf, count, 0);
        Socket *socket = RDMA_Socket(root);
        send_(socket, msg);
        AMessage_destroy(msg);
    }
}

int RDMA_Allreduce(const void *sendbuf, void *recvbuf,
                   int count, int datatype, int op, int rdma_group)
{
    int local_rank = RDMA_Rank();
    if (local_rank == 0)
    {
        RDMA_Reduce(sendbuf, recvbuf, count, datatype, op, 0, rdma_group);
        RDMA_MakeAll(recvbuf, count, 0, rdma_group);
    }
}