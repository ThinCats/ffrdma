#include "fmpi.hpp"
#include "process.hpp"
#include "utils/decoder.hpp"
#include "utils/rank_alg.hpp"
#include "utils/arg_parser.hpp"

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

int RDMA_Init(int argc, char **argv) {
  // parse arguments
  auto argObj = parseArgs(argc, argv);
  auto myIp = argObj.getValStr("r_myip");
  auto myPort = argObj.getValInt("r_myport");
  auto hostmapStr = argObj.getValStr("r_hostmap");

  // decode
  auto hostmap = ffrdma::Decoder::decode(hostmapStr);
  auto procInfoArr = ffrdma::RankAlg::calRank(hostmap);
  int myRank = ffrdma::RankAlg::getRank(procInfoArr, myIp, myPort);

  ffrdma::RdmaProcess::Init(ffrdma::RdmaProcessInfo(myRank, myIp, myPort), 
    procInfoArr
  );

  // test
  ffrdma::RdmaProcess::Instance();
  return 0;
}

int RDMA_Finalize() {
  ffrdma::RdmaProcess::Destroy();
  return 0;
}

int RDMA_Rank() {
  return ffrdma::RdmaProcess::Instance().myRank();
}

int RDMA_Size() {
  return ffrdma::RdmaProcess::Instance().worldSize();
}

Socket *const RDMA_Socket(int toRank) {
  return ffrdma::RdmaProcess::Instance().getSocket(toRank);
}

Socket *const RDMA_Reconnect(int toRank) {
  return ffrdma::RdmaProcess::Instance().reconnect(toRank);
}