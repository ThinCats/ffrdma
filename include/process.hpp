#ifndef FFRDMASOCKET_HPP
#define FFRDMASOCKET_HPP
#include "utils/socket.hpp"
#include "utils/types.hpp"

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
  Socket* const reconnect(int toRank);

  ~RdmaProcess();

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
  void setup();

  void setupAsServer();

  void setupAsClient();

  // Accept in single thread - block, accept all that rank is smaller than me
  void setupAccept();

  Socket *reconnectAsServer(int fromRank);

  Socket *reconnectAsClient(int toRank);

  Socket *acceptFromClient(int fromRank);

// helper function
  Socket* connetToServer(int toRank);

  RankType rankType(int rank);

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
} // namespace ffrdma
#endif // FFRDMASOCKET_HPP