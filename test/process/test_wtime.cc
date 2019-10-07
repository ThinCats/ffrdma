#include "mpi.hpp"
#include <stdio.h>
#include <thread>

int main(int argc, char **argv) {
  RDMA_Init(&argc, &argv);
  double start = RDMA_Wtime();
  printf("Start: %f\n", start);
  std::this_thread::sleep_for(std::chrono::seconds(2));
  double end = RDMA_Wtime();
  printf("End: %f\n", end);
  double elapsed = end - start;
  printf("Elapsed: %f\n", elapsed);
  RDMA_Finalize();
}