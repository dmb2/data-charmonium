#include "root-sugar.hh"
std::vector<std::string> split_string(const std::string& input, const std::string& delims ){
  std::vector<std::string> result;
  char *tok;
  char *buff = new char [input.size()];
  strncpy(buff,input.c_str(),input.size());
  tok = strtok(buff,delims.c_str());
  while(tok){
    // MSG_DEBUG(tok);
    if(tok){
      result.push_back(tok);
    }
    tok = strtok(NULL,delims.c_str());
  }
  if(buff) delete buff;
  return result;
}


