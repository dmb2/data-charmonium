#include "root-sugar.hh"
#include <sstream>
std::vector<std::string> split_string(const std::string& input, const char& delim){
  std::vector<std::string> result;
  std::stringstream ss(input);
  std::string item;
  while(std::getline(ss,item,delim)){
    // MSG_DEBUG(item);
    result.push_back(item);
  }
  return result;
}


