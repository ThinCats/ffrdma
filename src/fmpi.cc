#include "fmpi.hpp"
#include "mpi.hpp"
#include "process.hpp"
#include "utils/arg_parser.hpp"
#include "utils/debug.hpp"
#include "utils/decoder.hpp"
#include "utils/rank_alg.hpp"
#include <chrono>
#include <stdexcept>
#include <string.h>

thlib::ArgObj parseArgs(int argc, char **argv) {
  thlib::ArgParser argParser("Rdma Run", "r_help");
  argParser.add_argument("r_myip", thlib::ArgType::T_STRING, true, "",
                         "The ip address of this process");
  argParser.add_argument("r_myport", thlib::ArgType::T_INT, true, "",
                         "The port of this process");
  argParser.add_argument("r_hostmap", thlib::ArgType::T_STRING, true, "",
                         "The hostmap for all clients");
  return argParser.parse_args(argc, argv);
}

int RDMA_Init(int *argc, char ***argv) {
  // parse arguments
  auto argObj = parseArgs(*argc, *argv);
  auto myIp = argObj.getValStr("r_myip");
  auto myPort = argObj.getValInt("r_myport");
  auto hostmapStr = argObj.getValStr("r_hostmap");
  // filter used args
  *argc -= argObj.getLastParsingIndex();
  *argv += argObj.getLastParsingIndex();
  printf("Argc: %d\n", *argc);
  printf("Argv: ");
  for (int i = 0; i < *argc; i++) {
    printf("%s ", (*argv)[i]);
  }
  printf("\n");

  int myRank;
  ffrdma::RankRdmaProcessInfoArray procInfoArr;
  // decode
  try {
    auto hostmap = ffrdma::Decoder::decode(hostmapStr);
    procInfoArr = ffrdma::RankAlg::calRank(hostmap);
    myRank = ffrdma::RankAlg::getRank(procInfoArr, myIp, myPort);
  } catch (std::invalid_argument &t) {
    std::cout << "Ip Port Format Error: " << t.what() << std::endl;
    std::exit(-1);
  }

  ffrdma::RdmaProcess::Init(ffrdma::RdmaProcessInfo(myRank, myIp, myPort),
                            procInfoArr);

  // test
  ffrdma::RdmaProcess::Instance();
  return 0;
}

int RDMA_Finalize() {
  ffrdma::RdmaProcess::Destroy();
  return 0;
}

int RDMA_Rank(RDMA_Comm comm) {
  return ffrdma::RdmaProcess::Instance().commMyRank(comm);
}

int RDMA_Size(RDMA_Comm comm) {
  return ffrdma::RdmaProcess::Instance().commWorldSize(comm);
}

double RDMA_Wtime() {
  static auto firstTime = std::chrono::high_resolution_clock::now();
  auto now = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::duration<double>>(now -
                                                                   firstTime)
      .count();
}

Socket *const RDMA_Socket(int toRank, RDMA_Comm comm) {
  return ffrdma::RdmaProcess::Instance().commGetSocket(toRank, comm);
}

Socket *const RDMA_Reconnect(int toRank, RDMA_Comm comm) {
  return ffrdma::RdmaProcess::Instance().reconnect(toRank, comm);
}

int RDMA_Comm_split(RDMA_Comm fromComm, int color, int key,
                    RDMA_Comm *newcomm) {
  assert(key >= 0);
  assert((color >= 0) || (color == RDMA_UNDEFINED));

  if (fromComm < 0 || newcomm == nullptr) {
    return RDMA_ERR_COMM;
  }

  int commSize = RDMA_Size(fromComm);

  constexpr int sendcount = 2;
  void *sendbuf = malloc(sendcount * sizeof(int));
  int p[] = {color, key};
  memcpy(sendbuf, p, sendcount * sizeof(int));
  // send {color, key} {color, key}... (int, int)

  constexpr int recvcount = sendcount;
  void *recvbuf = malloc(recvcount * commSize * sizeof(int));
  int status = RDMA_Allgather_exp(sendbuf, sendcount, R_TYPE_INT, recvbuf,
                                  recvcount, R_TYPE_INT, fromComm);
  if (status != 0) {
    printf("%d\n", status);
    return RDMA_INTERN;
  }

  // assemble to vector
  std::vector<ffrdma::CommConfig> configs;
  configs.reserve(commSize);
  int recvbufSize = commSize * sendcount;
  for (int i = 0; i < recvbufSize; i += 2) {
    configs.push_back({
        ((int *)recvbuf)[i],
        ((int *)recvbuf)[i + 1],
    });
  }

  // split comm in process
  *newcomm =
      ffrdma::RdmaProcess::Instance().splitComm(fromComm, std::move(configs));

  printf("fromcomm: %d, newcomm: %d\n", fromComm, *newcomm);
  RDMA_Barrier(fromComm);

  return RDMA_SUCCESS;
}

int RDMA_Comm_free(RDMA_Comm *comm) {
  ffrdma::RdmaProcess::Instance().freeComm(comm);
  return RDMA_SUCCESS;
}