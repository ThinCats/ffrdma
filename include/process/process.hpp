#ifndef FFRDMASOCKET_HPP
#define FFRDMASOCKET_HPP
#include "../utils/socket.hpp"
#include "../utils/types.hpp"

#include <stdexcept>
#include <string>
#include <vector>

namespace ffrdma {

class RdmaProcess {
public:
  RdmaProcess(const RdmaProcessInfo &myInfo,
              const RankRdmaProcessInfoArray &arr)
      : myRank(myInfo.rank), myIp(myInfo.ip), myPort(myInfo.port),
        m_procInfos(arr), m_socketPool(arr.size(), nullptr) {}

  size_t worldSize() const { return this->m_procInfos.size(); }
  size_t rank() const { return this->myRank; }
  Socket_ *getSocket(int rank) {
#ifdef FFRDMA_ERROR_CHECK
    if (rank == myRank) {
      throw std::logic_error("can't get socket which is the same as my rank");
    }
    if (rank < 0 || rank >= worldSize()) {
      throw std::overflow_error("rank is overflow: " + std::to_string(rank));
    }
#endif
    return m_socketPool[rank];
  }

  // Must call, to make constructor exception safe
  void setup() {
    setupAsServer();
    setupAsClient();
  }

private:
  void setupAsServer() {
    if (m_state != INIT) {
      throw std::logic_error("only can open server socket when is initial");
    }
    // build server listen socket
    // backlog is the max size of clients can wait
    int backlog = worldSize() - myRank - 1;
    m_listenSocket = Socket::createServerSocket(myIp, myPort, backlog);
    m_state = OPEN_PORT;
  }

  void setupAsClient() {
    if (m_state != OPEN_PORT) {
      throw std::logic_error(
          "should create server socket before connect to other client");
    }

    // connect to all server with rank greater the me
    for (int rank = myRank + 1; rank < worldSize(); rank++) {
      auto &pi = m_procInfos[rank];
      auto retPair = Socket::createConnectToServer(pi.ip, pi.port, pi.rank);
      // add to socket list
      if (retPair.second == Socket::ConnStatus::OK) {
        m_socketPool[rank] = retPair.first;
      } else {
        // TODO: Implement
        throw std::logic_error("Not implement");
      }
    }
    m_state = CONNECTED_TO_ALL_SERVER;
  }

private:
  enum State {
    INIT,
    OPEN_PORT,
    CONNECTED_TO_ALL_SERVER,
    ACCEPT,
  };

private:
  int myRank;
  std::string myIp;
  int myPort;

  RankRdmaProcessInfoArray m_procInfos;
  Socket_ *m_listenSocket;
  // connect sockets, connect to other server or recv by clients
  std::vector<Socket_ *> m_socketPool;
  State m_state = INIT;
};

} // namespace ffrdma
#endif // FFRDMASOCKET_HPP