#include <iostream>
#include "include/utils/arg_parser.hpp"
#include "include/utils/strings.hpp"

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

int main(int argc, char **argv) {
  auto argObj = parseArgs(argc, argv);

  auto hostmap = argObj.getValStr("r_hostmap");
  std::cout << hostmap << std::endl;
}
