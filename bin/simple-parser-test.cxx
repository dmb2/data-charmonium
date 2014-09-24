#include <iostream>
#include "simple-parser.hh"
void usage(const char* prog_name){
  std::cout <<"Usage: "<<prog_name<< " test_config.conf"<<std::endl;
}
int main(const int argc, const char* argv[]){
  if(argc != 2) {
    usage(argv[0]);
    return 1;
  }
  std::cout << "Testing config: "<<argv[1]<<std::endl;
  std::ifstream file(argv[1]);
  parse_file(file);
  return 0;
}
