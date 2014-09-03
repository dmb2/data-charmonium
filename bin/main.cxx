#include <cstdlib>
#include <iostream>
#include "cut-flow-studies.hh"
#include <vector>
#include <string>

int main(const int argc, const char* argv[]){
  //Parse command line arguments
  std::vector<std::string> arg_list;
  for(int i=0; i < argc; i++){
    arg_list.push_back(std::string(argv[i]));
    // std::cout<<"Got argument: "<<argv[i]<<std::endl;
  }
  TTree* tree=NULL;
  process_tree(tree);
  
  return 0;
}
