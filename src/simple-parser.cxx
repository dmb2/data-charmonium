#include <iostream>
#include <sstream>

#include "simple-parser.hh"

int parse_file(std::ifstream& file, std::map<std::string,std::string>& raw_config){
  std::string line;
  while(std::getline(file,line)){
    std::istringstream line_stream;
    std::string key;
    if(std::getline(line_stream,key,'=')){
      std::string value;
      if(std::getline(line_stream,value)){
	raw_config[key]=value;
      }
      else{
	std::cerr << "Error parsing value of key: "<<key << std::endl;
	return VALUE_PARSE_ERROR;
      }
    }
    else{
      std::cerr <<"Error parsing key of line: "<<line<<std::endl;
      return KEY_PARSE_ERROR;
    }
  }
  return 0;
}
