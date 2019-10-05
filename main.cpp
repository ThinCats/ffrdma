#include <iostream>
#include "include/utils/arg_parser.hpp"
#include "include/utils/strings.hpp"



int main(int argc, char **argv) {
  auto argObj = parseArgs(argc, argv);

  auto hostmap = argObj.getValStr("r_hostmap");
  std::cout << hostmap << std::endl;
}
