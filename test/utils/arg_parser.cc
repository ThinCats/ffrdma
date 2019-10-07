#include "mpi.hpp"

int main(int argc, char **argv) {
  RDMA_Init(&argc, &argv);
  return 0;
}