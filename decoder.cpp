
#define FFRDMA_DEBUG
#include "include/utils/decoder.hpp"
#include "include/utils/rank_alg.hpp"
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

int main() {
  rank_test();
  return 0;
}