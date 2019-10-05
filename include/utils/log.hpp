#ifndef LOG_HPP
#define LOG_HPP
#include <string>

namespace ffrdma {
namespace log {
// template <typename... Args> std::string toString(Args &&... args) {
// }

std::string toString(int val) { return std::to_string(val); }
template <typename Head> std::string toString(Head &&head) {
  return std::string(head);
}
template <typename Head, typename... Args>
std::string toString(Head &&head, Args &&... args) {
  return toString(head) + " " + toString(args...);
}
} // namespace log
} // namespace ffrdma
#endif // LOG_HPP