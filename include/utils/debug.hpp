#ifndef DEBUG_HPP
#define DEBUG_HPP
#include "types.hpp"
#include <ostream>
#include <vector>

#define FFRDMA_DEBUG
#ifdef FFRDMA_DEBUG
#  include "prettyprint.hpp"
#endif

namespace ffrdma {
std::ostream &operator<<(std::ostream &out, const RdmaProcessInfo &proc) {
  out << "rank: " << proc.rank << " port: " << proc.port << " ip: " << proc.ip << std::endl;
  return out;
}
} // namespace ffrdma

#endif // DEBUG_HPP