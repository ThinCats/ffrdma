#ifndef COMM_HPP
#define COMM_HPP
#include <vector>
#ifdef FFRDMA_DEBUG
#  include <iostream>
#endif

const static int RDMA_UNDEFINED = -1;

// RDMA_Comm represents the comm type, 0 is the world_Comm
typedef int RDMA_Comm;

const static RDMA_Comm RDMA_Comm_Root = 0;

namespace ffrdma {

struct CommConfig {
  int color;
  int key;
#ifdef FFRDMA_DEBUG
  friend std::ostream &operator<<(std::ostream &out, const CommConfig &rhs) {
    out << rhs.color << " " << rhs.key << std::endl;
    return out;
  }
#endif
};

struct MemCommConfig {
  // the acutal rank
  int rootRank;
  int key;

  bool operator<(const MemCommConfig &rhs) { return this->key < rhs.key; }

#ifdef FFRDMA_DEBUG
  friend std::ostream &operator<<(std::ostream &out, const MemCommConfig &rhs) {
    out << rhs.rootRank << " " << rhs.key << std::endl;
    return out;
  }
#endif
};

// All index is refer to the root world rank
struct RDMA_Communicator {
  RDMA_Communicator(int worldSize)
      : commRanks(worldSize, -1), rootRanks(worldSize, -1) {}

  int color = 0;
  int myCommRank = 0;
  int commWorldSize = 0;
  // current rank for current comm
  // rootRank -> commRank
  mutable std::vector<int> commRanks;
  // commRank -> rootRank
  mutable std::vector<int> rootRanks;

  void setRank(int rootRank, int commRank) {
    commRanks[rootRank] = commRank;
    rootRanks[commRank] = rootRank;
  }

  bool isMem(int rootRank) const { return commRanks[rootRank] != -1; }
};
};     // namespace ffrdma
#endif // COMM_HPP