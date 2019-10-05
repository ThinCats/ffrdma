
#define FFRDMA_DEBUG
#include "include/utils/decoder.hpp"
#include "include/utils/rank_alg.hpp"
#include "include/process/process.hpp"
#include "include/utils/debug.hpp"
#include <iostream>

int splitter() {
  auto sp = ffrdma::Spliter("good|notice|you", "|");

  while (!sp.end()) {
    std::cout << sp.getNext() << std::endl;
  }
  return 0;
}

void decoder_test() {
  auto data = "10.10.10.10:400+200+300+400,10.10.10.11:500+600+700+800,10.10.10.12:800+900";
  auto hostmap = ffrdma::Decoder::decode(data);
  std::cout << hostmap;
}

void rank_test() {
  auto data = "10.10.10.10:400+200+300+400,10.10.10.11:500+600+700+800,10.10.10.12:800+900";
  auto hostmap = ffrdma::Decoder::decode(data);
  auto procInfoArr = ffrdma::RankAlg::calRank(hostmap);
  std::cout << procInfoArr << std::endl;
}

void process_test() {
  auto data = "10.10.10.10:400+200+300+400,10.10.10.11:500+600+700+800,10.10.10.12:800+900";
  auto myIp = "10.10.10.10";
  auto myPort = 400;
  auto hostmap = ffrdma::Decoder::decode(data);
  auto procInfoArr = ffrdma::RankAlg::calRank(hostmap);

  // get my addr
  auto myRank = ffrdma::RankAlg::getRank(procInfoArr, myIp, myPort);
  ffrdma::RdmaProcessInfo myRankInfo = {myRank, myIp, myPort};
  // INIT
  ffrdma::RdmaProcess::Init(myRankInfo, procInfoArr);
  auto &process = ffrdma::RdmaProcess::Instance();
  std::cout << &process << std::endl;
  std::cout << process.getSocket(3) << std::endl;
  // process.reconnect(3);
}

int main() {
  // rank_test();
  process_test();
  return 0;
}