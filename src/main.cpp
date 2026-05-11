#include <iostream>

#include "FileTape.hpp"
#include "TapeConfig.hpp"
#include "TapeSorter.hpp"

int main(int argc, char **argv) {
  if (argc != 3) {
    std::cout << "Usage: " << argv[0] << " <input file> <output file>\n";
    return 1;
  }
  try {
    TapeConfig config = TapeConfig::load("config.txt");
    FileTape::setGlobalConfig(config);
    FileTape input(argv[1], OpenMode::kOpen);
    FileTape output(argv[2], OpenMode::kCreate);
    TapeSorter::Sort(input, output);
  } catch (std::exception &e) {
    std::cerr << e.what() << "\n";
    return 1;
  }

  return 0;
}