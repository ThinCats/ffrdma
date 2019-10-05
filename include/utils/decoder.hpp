#ifndef SERIALIZER_HPP
#define SERIALIZER_HPP
#include "types.hpp"
#include <map>
#include <set>
#include <string>
#include <vector>
#include <iostream>

namespace ffrdma {

class Spliter {
  enum State { START, END };

public:
  /**
   * @brief Construct a new Spliter object
   * @param data
   * @param delim
   * @param warningOnOverflow - if set true, throw exception when call getNext
   * after end
   */
  Spliter(const std::string &data, const std::string &delim,
          bool warningOnOverflow = false)
      : m_data(data), m_delim(delim), overflowWarn(warningOnOverflow) {}

  std::string getNext() {
    if (m_state == END) {
      if (overflowWarn) {
        throw std::overflow_error("Spliter for " + m_data + " overflow");
      }
      return "";
    }

    std::string token;

    if ((pos_end = m_data.find(m_delim, pos_start)) != std::string::npos) {
      token = m_data.substr(pos_start, pos_end - pos_start);
      pos_start = pos_end + m_delim.size();
      return token;
    }

    // END
    m_state = END;

    return m_data.substr(pos_start);
  }

  bool end() const { return m_state == END; }

private:
  const std::string &m_data;
  std::string m_delim;
  // inner state
  State m_state = START;
  std::size_t pos_start = 0, pos_end = 0;
  // config
  bool overflowWarn;
};

// 10.10.10.10:123+456+789,10.10.10.11:456+879+123
class Decoder {
  using StringList = std::vector<std::string>;
  using IntList = std::vector<int>;
  using addrPair = std::pair<std::string, std::string>;

public:
  static HostPortsMap decode(const std::string &data) {
    auto sp = Spliter(data, ",");
    auto hostmap = HostPortsMap();

    while (!sp.end()) {
      auto addrPair = decodeAddrPair(sp.getNext());
      auto host = addrPair.first;
      auto ports = decodePortList(addrPair.second);
      auto retPair =
          hostmap.insert({host, std::set<int>(ports.begin(), ports.end())});

      if (retPair.second == false) {
        // already exist one, duplicate specified
        throw std::invalid_argument("Duplicate hostname");
      }
    }
    return hostmap;
  }

private:
  static addrPair decodeAddrPair(const std::string &hostStr) {
    auto sp = Spliter(hostStr, ":");
    // TODO: handle Error
    auto host = sp.getNext();
    auto portsStr = sp.getNext();
    return {host, portsStr};
  }

  // 100+200+300
  static IntList decodePortList(const std::string &portsStr) {
    IntList ports;
    auto sp = Spliter(portsStr, "+");
    while (!sp.end()) {
      auto portStr = sp.getNext();
      // TODO: handle Error
      auto port = std::stoi(portStr);
      ports.push_back(port);
    }
    return ports;
  }
};
} // namespace RDMA
#endif // SERIALIZER_HPP