#ifndef FFRDMASOCKET_HPP
#define FFRDMASOCKET_HPP
#include "comm.hpp"
#include "utils/socket.hpp"
#include "utils/types.hpp"

#include <cassert>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

namespace ffrdma {

struct RetryPolicy {
  // ums
  using milliSecond = std::chrono::duration<int, std::ratio<1, 1000>>;
  enum Policy {
    // Never retry
    Never,
    // Always retry
    Always,
    // Try N times then failed
    TryFailed
  };
  RetryPolicy(Policy p, int c = 0, const milliSecond &st = milliSecond(0))
      : policy(p), retryTimes(c), sleepTime(st) {}

  Policy policy;
  int retryTimes = 0;
  milliSecond sleepTime;
};

class RdmaProcess {
public:
  /**
   * @brief - the role in a rank
   */
  enum RankType { Server, Client, Self };

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

#ifdef FFRDMA_ERROR_CHECK
  size_t commMyRank(RDMA_Comm comm) const {
    return m_commPool.at(comm).myCommRank;
  }
  size_t commWorldSize(RDMA_Comm comm) const {
    return m_commPool.at(comm).commWorldSize;
  }
  Socket *const commGetSocket(int rank, RDMA_Comm comm) const {
    rank = getRootRank(comm, rank);
    if (rank == m_myRootRank) {
      throw std::logic_error("can't get socket which is the same as my rank");
    }
    if (rank < 0 || rank >= worldSize()) {
      throw std::overflow_error("rank is overflow: " + std::to_string(rank));
    }
    return m_socketPool[rank];
  }
#else
  size_t commMyRank(RDMA_Comm comm) const {
    return m_commPool[comm].myCommRank;
  }
  size_t commWorldSize(RDMA_Comm comm) const {
    return m_commPool[comm].commWorldSize;
  }
  Socket *const commGetSocket(int rank, RDMA_Comm comm) const {
    return m_socketPool[getRootRank(comm, rank)];
  }
#endif

  // TODO: More elegant way
  Socket *const reconnect(int toRank, RDMA_Comm comm);

  /**
   * @brief
   * Split comm according to the configs (contains all the rank and color)
   * only choose those same as myColor, rerank with key and myKey  *
   * @param from - from which comm to split
   * @param commConfigList - a list of configs [color, key], the index is the
   * commRank (view as from comm)
   * @return RDMA_Comm - return the new Comm it creates
   */
  RDMA_Comm splitComm(RDMA_Comm from, std::vector<CommConfig> &&commConfigList);

  void freeComm(RDMA_Comm *comm);

  ~RdmaProcess();

private:
  RdmaProcess(const RdmaProcessInfo &myInfo,
              const RankRdmaProcessInfoArray &arr)
      : m_myRootRank(myInfo.rank), m_myIp(myInfo.ip), m_myPort(myInfo.port),
        m_procInfos(arr), m_socketPool(arr.size(), nullptr) {}

  RdmaProcess() {}

  RdmaProcess(const RdmaProcess &) = delete;

  // !! @deprected
  // Init RdmaProcess by configs
  void init(const RdmaProcessInfo &myInfo,
            const RankRdmaProcessInfoArray &arr) {
    m_myRootRank = myInfo.rank;
    m_myIp = myInfo.ip;
    m_myPort = myInfo.port;
    m_procInfos = arr;
    m_socketPool.resize(arr.size());
  }

  // Singleton instance
  static std::unique_ptr<RdmaProcess> process;

private:
  size_t worldSize() const { return this->m_procInfos.size(); }
  // Must call, to make constructor exception safe
  void setup();

  void setupRootCommWorld();

  void setupAsServer();

  void setupAsClient();

  // Accept in single thread - block, accept all that rank is smaller than me
  void setupAccept();

  Socket *reconnectAsServer(int fromRank);

  Socket *reconnectAsClient(int toRank);

  Socket *acceptFromClient(int fromRank);

  // helper function
  Socket *connectToServer(int toRank, RetryPolicy policy);

  RankType rankType(int rank);

  int getRootRank(RDMA_Comm comm, int rank) const {
#ifdef FFRDMA_ERROR_CHECK
    return m_commPool.at(comm).rootRanks[rank];
#else
    return m_commPool[comm].rootRanks[rank];
#endif
  }

private:
  enum State {
    INIT,
    OPEN_PORT,
    CONNECTED_TO_ALL_SERVER,
    ACCEPT,
  };

private:
  int m_myRootRank;

  std::string m_myIp;
  int m_myPort;
  RankRdmaProcessInfoArray m_procInfos;
  Socket *m_listenSocket;
  // connect sockets, connect to other server or recv by clients
  std::vector<Socket *> m_socketPool;
  State m_state = INIT;

  std::vector<RDMA_Communicator> m_commPool;
};
} // namespace ffrdma
#endif // FFRDMASOCKET_HPP