#ifndef RANK_ALG_HPP
#define RANK_ALG_HPP

#include "types.hpp"

namespace ffrdma {
/**
 * @brief Define the algorithm to calculate Rank for each process
 */
struct RankAlg {
  /**
   * @brief calculate rank
   * @param m - the ports map
   * @return RankRdmaProcessInfoArray - the result
   */
  static RankRdmaProcessInfoArray calRank(const HostPortsMap &m) {
    RankRdmaProcessInfoArray procArr;
    // rank cnt
    int rank = 0;

    // by default sequence (use map and set)
    for (auto &hostPorts : m) {
      for (auto &port : hostPorts.second) {
        procArr.push_back({rank, hostPorts.first, port});
        rank++;
      }
    }
    return procArr;
  }
};

} // namespace ffrdma

#endif // RANK_ALG_HPP