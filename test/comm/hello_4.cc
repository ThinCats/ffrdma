#include "mpi.hpp"
#include "rdma_socket.h"
#include <stdio.h>

void printComm(RDMA_Comm comm) {
  printf("\nI am rank: %d, in comm %d with size %d\n", RDMA_Rank(comm),
         comm, RDMA_Size(comm));
}

int main(int argc, char **argv) {
  RDMA_Init(argc, argv);

  const int splitSize = 2;

  int comm = 0;
  int next_comm;
  int color;
  while(RDMA_Size(comm) > 1) {
    printComm(comm);
    // split
    color = RDMA_Rank(comm) % splitSize;
    int status = RDMA_Comm_split(comm, color, RDMA_Rank(comm), &next_comm);
    if (status != 0) {
      printf("Error in split: %d\n", status);
      break;
    }
    printf("Continue: ?");
    getchar();
    comm = next_comm;
  }
  // getchar();
  RDMA_Finalize();
}