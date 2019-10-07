#include "process.hpp"
#include "rdma_socket.h"
#include "utils/log.hpp"
#include <algorithm>
#include <iostream>
#include <thread>


// TODO: More elegant way
Socket *const ffrdma::RdmaProcess::reconnect(int toRank, RDMA_Comm comm) {
  toRank = getRootRank(comm, toRank);
  switch (rankType(toRank)) {
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

RDMA_Comm
ffrdma::RdmaProcess::splitComm(RDMA_Comm from,
                               std::vector<CommConfig> &&commConfigList) {

  if (from < 0 || from >= m_commPool.size()) {
    throw std::invalid_argument("comm is out of range");
  }

  int myFromCommRank = m_commPool[from].myCommRank;
  int myColor = commConfigList[myFromCommRank].color;

  // <rootRank, key>
  std::vector<MemCommConfig> memCommConfigs;

  // RDMA_UNDEFINED, only create a comm with single process
  if (myColor == RDMA_UNDEFINED) {
    // Only me in this comm
    memCommConfigs.push_back({m_commPool[from].rootRanks[myFromCommRank], 0});
  } else {
    // get mem configs
    for (size_t cRank = 0; cRank < commConfigList.size(); cRank++) {
      auto &config = commConfigList[cRank];
      if (config.color == myColor) {
        memCommConfigs.push_back(
            {m_commPool[from].rootRanks[cRank], config.key});
      }
    }

    // calculate new rank
    std::sort(memCommConfigs.begin(), memCommConfigs.end());
  }

  // create a new comm
  RDMA_Communicator newComm(worldSize());
  newComm.color = myColor;
  newComm.commWorldSize = memCommConfigs.size();

  for (size_t nRank = 0; nRank < memCommConfigs.size(); nRank++) {
    int rootRank = memCommConfigs[nRank].rootRank;
    newComm.setRank(rootRank, nRank);
  }
  newComm.myCommRank = newComm.commRanks[m_myRootRank];

  // push to pool
  m_commPool.push_back(newComm);
  return RDMA_Comm(m_commPool.size() - 1);
}

// !! TODO: Implement freeComm
void ffrdma::RdmaProcess::freeComm(RDMA_Comm *comm) {
#ifdef FFRDMA_CHECK_ERROR
  if (comm == nullptr) {
    throw std::invalid_argument("can't free nullptr comm");
  }
  if (comm == 0) {
    throw std::invalid_argument("can't free root comm");
  }
#endif
  *comm = -1;
}

ffrdma::RdmaProcess::~RdmaProcess() {
  // !! The server socket will never close, because it can be closed by clients
  // !! Or it will close twice
  // !! FIX: If the client is disconnected from network
  if (m_listenSocket != nullptr) {
    // close_(m_listenSocket);
  }
  for (Socket *socket : m_socketPool) {
    if (socket != nullptr) {
      close_(socket);
    }
  }
}

void ffrdma::RdmaProcess::setup() {
  setupRootCommWorld();
  setupAsServer();
  setupAsClient();
  setupAccept();
}

void ffrdma::RdmaProcess::setupRootCommWorld() {
  auto comm = RDMA_Communicator(worldSize());
  comm.color = 0;
  comm.myCommRank = m_myRootRank;
  comm.commWorldSize = worldSize();
  // init nowRank
  for (int rank = 0; rank < worldSize(); rank++) {
    comm.setRank(rank, rank);
  }
  m_commPool.push_back(comm);
}

void ffrdma::RdmaProcess::setupAsServer() {
  assert(m_state == INIT);
  // Rank 0 should not have listening socket
  if (m_myRootRank != 0) {
    // build server listening socket
    // backlog is the max size of clients can wait
    int backlog = m_myRootRank;
    m_listenSocket = utils::createServerSocket(m_myIp, m_myPort, backlog);
  }
  m_state = OPEN_PORT;
}

void ffrdma::RdmaProcess::setupAsClient() {
  assert(m_state == OPEN_PORT);
  // connect to all server with rank greater the me
  for (int rank = m_myRootRank + 1; rank < worldSize(); rank++) {
    Socket *connSocket =
        connectToServer(rank, RetryPolicy(RetryPolicy::TryFailed, 10,
                                          RetryPolicy::milliSecond(1000)));
    m_socketPool[rank] = connSocket;
  }
  m_state = CONNECTED_TO_ALL_SERVER;
}

// Accept in single thread - block, accept all that rank is smaller than me
void ffrdma::RdmaProcess::setupAccept() {
  assert(m_state == CONNECTED_TO_ALL_SERVER);

  for (int rank = 0; rank < m_myRootRank; rank++) {
    // accept all from client, block
    Socket *newSocket = acceptFromClient(rank);
    // should equal to this rank
    assert(rankType(newSocket->node_id) == Client);
    m_socketPool[newSocket->node_id] = newSocket;
  }
  m_state = ACCEPT;
}

Socket *ffrdma::RdmaProcess::reconnectAsServer(int fromRank) {
  // reaccept once
  auto listen = acceptFromClient(fromRank);
  if (listen->node_id != fromRank) {
    throw std::invalid_argument(
        log::toString("ReconnectAsServer: In knodeId and excepted Rank not "
                      "matched:",
                      "nodeId", listen->node_id, "expect rank", fromRank));
  }
  // update socket pool
  close_(m_socketPool[fromRank]);
  m_socketPool[fromRank] = listen;
  return listen;
}

Socket *ffrdma::RdmaProcess::reconnectAsClient(int toRank) {
  Socket *socket =
      connectToServer(toRank, RetryPolicy(RetryPolicy::TryFailed, 5,
                                          RetryPolicy::milliSecond(10)));
  // update socket pool
  close_(m_socketPool[toRank]);
  m_socketPool[toRank] = socket;
  return socket;
}

Socket *ffrdma::RdmaProcess::acceptFromClient(int fromRank) {
  auto listen = accept_(m_listenSocket, nullptr);

  if (listen == nullptr) {
    throw std::runtime_error("Accept failed from: " +
                             utils::ipPortString(m_procInfos[fromRank].ip,
                                                 m_procInfos[fromRank].port));
  }
  return listen;
}

// helper function
Socket *ffrdma::RdmaProcess::connectToServer(int toRank, RetryPolicy policy) {
  auto &pi = m_procInfos[toRank];
  int retryCnt = 0;
  while (true) {
    auto retPair = utils::createConnectToServer(pi.ip, pi.port, m_myRootRank);
    // add to socket list
    auto connStatus = retPair.second;
    if (connStatus == utils::ConnStatus::OK) {
      return retPair.first;
    } else if (connStatus == utils::ConnStatus::CONN_ERROR) {
      if (policy.policy == RetryPolicy::Never) {
        // no retry
        return retPair.first;
      }
      if (policy.policy == RetryPolicy::TryFailed &&
          retryCnt >= policy.retryTimes) {
        // policy retry for some times
        std::cout << "Retry failed after " << retryCnt << " End." << std::endl;
        throw std::invalid_argument(
            log::toString("Connect to server failed:", pi.ip, pi.port));
      }
      std::cout << "Retry: " << retryCnt << " Sleep for "
                << policy.sleepTime.count() << "ms" << std::endl;
      retryCnt++;
      std::this_thread::sleep_for(policy.sleepTime);
    } else {
      // TODO: Implement
      throw std::logic_error("Not implement");
    }
  }
  return nullptr;
}

ffrdma::RdmaProcess::RankType ffrdma::RdmaProcess::rankType(int rank) {
#ifdef FFRDMA_ERROR_CHECK
  if (rank < 0 || rank > worldSize()) {
    throw std::out_of_range("Rank is overflow");
  }
#endif
  if (rank < m_myRootRank) {
    return Client;
  } else if (rank > m_myRootRank) {
    return Server;
  } else {
    return Self;
  }
  return Self;
}

// Singleton instance
// !! warning, it's not thread safe
std::unique_ptr<ffrdma::RdmaProcess> ffrdma::RdmaProcess::process = nullptr;