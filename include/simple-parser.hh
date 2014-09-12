#pragma once
#include <fstream>
#include <map>
#include <string>

enum ERROR{KEY_PARSE_ERROR, VALUE_PARSE_ERROR, FILE_IO_ERROR};
int parse_file(std::ifstream& file, std::map<std::string,std::string>& raw_config);
