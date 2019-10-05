#ifndef TYPES_HPP
#define TYPES_HPP

#include <map>
#include <ostream>
#include <set>
#include <vector>

namespace ffrdma {
struct RdmaProcessInfo {
  RdmaProcessInfo(int r, const std::string &i, int p):
    rank(r), ip(i), port(p)
   {}

  int rank;
  std::string ip;
  int port;
};

// 10.10.10.10 -> [100, 200, 300]
using HostPortsMap = std::map<std::string, std::set<int>>;
// 10.10.10.10:100 -> 0

// use rank as index
using RankRdmaProcessInfoArray = std::vector<RdmaProcessInfo>;
// #endif
} // namespace ffrdma
#endif // TYPES_HPP