#ifndef FFRDMA_MOCK_SOCKET
#include "utils/socket.hpp"
#include "rdma_socket.h"
#else
#include "rsocket_mock/rdma_socket.h"
#endif

std::string ffrdma::utils::ipPortString(const std::string &ip, int port) {
  return ip + ":" + std::to_string(port);
}

// helper function, bind and listen
Socket *ffrdma::utils::createServerSocket(const std::string &ip, int port, int backlog) {
  sockaddr_in sockaddr;
  if (inet_pton(AF_INET, ip.c_str(), &sockaddr) != 1) {
    throw std::invalid_argument("Failed to read net ip: " + ip);
  }

  sockaddr.sin_family = AF_INET;
  sockaddr.sin_port = htons(port);
  memset(sockaddr.sin_zero, 0, 8);

  Socket *socket = socket_(RDMA_PS_TCP);

  if (bind_(socket, &sockaddr, AF_INET)) {
    free(socket);
    throw std::runtime_error("Bind to address failed: " +
                             ipPortString(ip, port));
  }

  // listen
  listen_(socket, backlog);

  return socket;
}

std::pair<Socket *, ffrdma::utils::ConnStatus> ffrdma::utils::createConnectToServer(const std::string ip,
                                                       int port, int nodeId) {
  Socket *socket = socket_(RDMA_PS_TCP);
  int status =
      connect_(&socket, ip.c_str(), std::to_string(port).c_str(), nodeId);
  if (status != 0) {
    free(socket);
    // throw std::runtime_error("connect to server: " + ipPortString(ip, port));
    return {nullptr, ConnStatus::CONN_ERROR};
  }

  return {socket, ConnStatus::OK};
}