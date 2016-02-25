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
std::string str_join(const std::string base, 
		const std::vector<std::string>& strings,
		const size_t start, const size_t end){
  if(start==end || strings.size()==0){
    return "";
  }
  std::string result(strings[start]);
  for(size_t i=(start+1); i < end; i++){
    result+=(base + strings[i]);
  }
  return result;
}
