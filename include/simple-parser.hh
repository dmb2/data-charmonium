#pragma once
#include <fstream>
#include <vector>
#include <string>

enum ERROR{KEY_PARSE_ERROR, VALUE_PARSE_ERROR, FILE_IO_ERROR};
int parse_file(std::ifstream& file,std::vector<std::vector<std::string > >& options);

