#pragma once
#include <fstream>
#include <vector>
#include <string>

#include "Cut.hh"

enum ERROR{KEY_PARSE_ERROR, VALUE_PARSE_ERROR, FILE_IO_ERROR};
int parse_file(std::ifstream& file,std::vector<std::vector<std::string > >& options);
void get_opts(const char* opt_fname,std::map<std::string,std::string>& value_opts,
	      real_cuts& CutDefReals, category_cuts& CutDefCats);
