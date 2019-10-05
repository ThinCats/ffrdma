#include "process.hpp"
#include "rdma_socket.h"

// TODO: More elegant way
Socket *const ffrdma::RdmaProcess::reconnect(int toRank) {
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

ffrdma::RdmaProcess::~RdmaProcess() {
  if (m_listenSocket != nullptr) {
    close_(m_listenSocket);
  }
  for (Socket *socket : m_socketPool) {
    if (socket != nullptr) {
      close_(socket);
    }
  }
}

void ffrdma::RdmaProcess::setup() {
  setupAsServer();
  setupAsClient();
  setupAccept();
}

void ffrdma::RdmaProcess::setupAsServer() {
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

void ffrdma::RdmaProcess::setupAsClient() {
  assert(m_state == OPEN_PORT);
  // connect to all server with rank greater the me
  for (int rank = m_myRank + 1; rank < worldSize(); rank++) {
    Socket *connSocket = connetToServer(rank);
    m_socketPool[rank] = connSocket;
  }
  m_state = CONNECTED_TO_ALL_SERVER;
}

// Accept in single thread - block, accept all that rank is smaller than me
void ffrdma::RdmaProcess::setupAccept() {
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

Socket *ffrdma::RdmaProcess::reconnectAsServer(int fromRank) {
  // reaccept once
  auto listen = acceptFromClient(fromRank);
  if (listen->node_id != fromRank) {
    throw std::invalid_argument(
        "ReconnectAsServer: nodeId and excepted Rank not matched");
  }
  // update socket pool
  close_(m_socketPool[fromRank]);
  m_socketPool[fromRank] = listen;
  return listen;
}

Socket *ffrdma::RdmaProcess::reconnectAsClient(int toRank) {
  Socket *socket = connetToServer(toRank);
  if (socket->node_id != toRank) {
    throw std::invalid_argument(
        "ReconnectAsClient: nodeId and excepted Rank not matched");
  }
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
Socket *ffrdma::RdmaProcess::connetToServer(int toRank) {
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

ffrdma::RdmaProcess::RankType ffrdma::RdmaProcess::rankType(int rank) {
#ifdef FFRDMA_ERROR_CHECK
  if (rank < 0 || rank > worldSize()) {
    throw std::overflow("Rank is overflow");
  }
#endif
  if (rank < m_myRank) {
    return Client;
  } else if (rank > m_myRank) {
    return Server;
  } else {
    return Self;
  }
}

// Singleton instance
// !! warning, it's not thread safe
std::unique_ptr<ffrdma::RdmaProcess> ffrdma::RdmaProcess::process = nullptr;