#ifndef FFRDMASOCKET_HPP
#define FFRDMASOCKET_HPP
#include "../utils/socket.hpp"
#include "../utils/types.hpp"

#include <cassert>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

namespace ffrdma {

class RdmaProcess {
public:
  /**
   * @brief - the role in a rank
   */
  enum RankType {
    Server,
    Client,
    Self
  };
public:
  /**
   * @brief Init Singleton rdma process with configs
   *  always called in init function
   * !! Warning, not thread safe
   * @param myInfo - the process info of current process
   * @param arr - the process info of other processes
   */
  static void Init(const RdmaProcessInfo &myInfo,
                   const RankRdmaProcessInfoArray &arr) {
    if (RdmaProcess::process != nullptr) {
      return;
    }
    RdmaProcess::process.reset(new RdmaProcess(myInfo, arr));
    RdmaProcess::process->setup();
  }

  /**
   * @brief destroy the singleton, only can call by Finalize function!
   * !! Not thread-safe
   */
  static void Destroy() {
    // should not call destroy twice
    assert(process != nullptr);
    RdmaProcess::process.reset(nullptr);
  }

  /**
   * @brief - singleton instance function
   *
   * @return const RdmaProcess& get const refrence for RdmaProcess
   */
  static RdmaProcess &Instance() {
    // should call Init before Instance
    assert(process != nullptr);
    return *process;
  }

  size_t worldSize() const { return this->m_procInfos.size(); }
  size_t myRank() const { return this->m_myRank; }
  Socket * const getSocket(int rank) const {
#ifdef FFRDMA_ERROR_CHECK
    if (rank == m_myRank) {
      throw std::logic_error("can't get socket which is the same as my rank");
    }
    if (rank < 0 || rank >= worldSize()) {
      throw std::overflow_error("rank is overflow: " + std::to_string(rank));
    }
#endif
    return m_socketPool[rank];
  }

  // TODO: More elegant way
  Socket* const reconnect(int toRank) {
    switch(rankType(toRank)) {
      case Server:
        return reconnectAsClient(toRank);
      break;
      case Client:
        return reconnectAsServer(toRank);
      break;
      default:
      throw std::invalid_argument("Reconnect Type Error");
    }
  }

  ~RdmaProcess() {
    if (m_listenSocket != nullptr) {
      close_(m_listenSocket);
    }
    for (Socket *socket : m_socketPool) {
      if (socket != nullptr) {
        close_(socket);
      }
    }
  }

private:
  RdmaProcess(const RdmaProcessInfo &myInfo,
              const RankRdmaProcessInfoArray &arr)
      : m_myRank(myInfo.rank), m_myIp(myInfo.ip), m_myPort(myInfo.port),
        m_procInfos(arr), m_socketPool(arr.size(), nullptr) {}

  RdmaProcess() {}

  RdmaProcess(const RdmaProcess&) = delete;

  // !! @deprected
  // Init RdmaProcess by configs
  void init(const RdmaProcessInfo &myInfo,
            const RankRdmaProcessInfoArray &arr) {
    m_myRank = myInfo.rank;
    m_myIp = myInfo.ip;
    m_myPort = myInfo.port;
    m_procInfos = arr;
    m_socketPool.resize(arr.size());
  }

  // Singleton instance
  static std::unique_ptr<RdmaProcess> process;

private:
  // Must call, to make constructor exception safe
  void setup() {
    setupAsServer();
    setupAsClient();
    setupAccept();
  }

  void setupAsServer() {
    assert(m_state == INIT);
    // Rank 0 should not have listening socket
    if (m_myRank != 0) {
      // build server listening socket
      // backlog is the max size of clients can wait
      int backlog = worldSize() - m_myRank - 1;
      m_listenSocket = utils::createServerSocket(m_myIp, m_myPort, backlog);
    }
    m_state = OPEN_PORT;
  }

  void setupAsClient() {
    assert(m_state == OPEN_PORT);
    // connect to all server with rank greater the me
    for (int rank = m_myRank + 1; rank < worldSize(); rank++) {
      Socket *connSocket = connetToServer(rank);
      m_socketPool[rank] = connSocket;
    }
    m_state = CONNECTED_TO_ALL_SERVER;
  }

  // Accept in single thread - block, accept all that rank is smaller than me
  void setupAccept() {
    assert(m_state == CONNECTED_TO_ALL_SERVER);

    for (int rank = 0; rank < m_myRank; rank++) {
      // accept all from client, block
      Socket *newSocket = acceptFromClient(rank);
      // should equal to this rank
      assert(newSocket->node_id == rank);
      m_socketPool[rank] = newSocket;
    }
    m_state = ACCEPT;
  }

  Socket *reconnectAsServer(int fromRank) {
    // reaccept once
    auto listen = acceptFromClient(fromRank);
    if (listen->node_id != fromRank) {
      throw std::invalid_argument("ReconnectAsServer: nodeId and excepted Rank not matched");
    }
    // update socket pool
    close_(m_socketPool[fromRank]);
    m_socketPool[fromRank] = listen;
    return listen;
  }

  Socket *reconnectAsClient(int toRank) {
    Socket *socket = connetToServer(toRank);
    if (socket->node_id != toRank) {
      throw std::invalid_argument("ReconnectAsClient: nodeId and excepted Rank not matched");
    }
    // update socket pool
    close_(m_socketPool[toRank]);
    m_socketPool[toRank] = socket;
    return socket;
  }

  Socket *acceptFromClient(int fromRank) {
    auto listen = accept_(m_listenSocket, nullptr);

      if (listen == nullptr) {
        throw std::runtime_error(
            "Accept failed from: " +
            utils::ipPortString(m_procInfos[fromRank].ip, m_procInfos[fromRank].port));
      }
    return listen;
  }

// helper function
  Socket* connetToServer(int toRank) {
    auto &pi = m_procInfos[toRank];
    auto retPair = utils::createConnectToServer(pi.ip, pi.port, m_myRank);
    // add to socket list
    if (retPair.second == utils::ConnStatus::OK) {
      return retPair.first;
    } else {
      // TODO: Implement
      throw std::logic_error("Not implement");
    }
  }

  RankType rankType(int rank) {
    #ifdef FFRDMA_ERROR_CHECK
    if (rank < 0 || rank > worldSize()) {
      throw std::overflow("Rank is overflow");
    }
    #endif
    if (rank < m_myRank) {
      return Client;
    }
    else if (rank > m_myRank){
      return Server;
    }
    else {
      return Self;
    }
  }

private:
  enum State {
    INIT,
    OPEN_PORT,
    CONNECTED_TO_ALL_SERVER,
    ACCEPT,
  };

private:
  int m_myRank;
  std::string m_myIp;
  int m_myPort;

  RankRdmaProcessInfoArray m_procInfos;
  Socket *m_listenSocket;
  // connect sockets, connect to other server or recv by clients
  std::vector<Socket *> m_socketPool;
  State m_state = INIT;
};

// Singleton instance
// !! warning, it's not thread safe
std::unique_ptr<RdmaProcess> RdmaProcess::process = nullptr;

} // namespace ffrdma
#endif // FFRDMASOCKET_HPP