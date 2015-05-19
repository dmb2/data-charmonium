#include <iostream>
#include "Cut.hh"
#include "simple-parser.hh"
void usage(const char* prog_name){
  std::cout <<"Usage: "<<prog_name<< " test_config.conf"<<std::endl;
}
int main(const int argc, const char* argv[]){
  if(argc != 2) {
    usage(argv[0]);
    return 1;
  }
  //TODO expand this to be a real unit test
  std::cout << "Testing config: "<<argv[1]<<std::endl;
  /*  std::ifstream file(argv[1]);
  std::vector<std::vector<std::string > > options;

  parse_file(file,options);
  for(std::vector<std::vector<std::string > >::const_iterator opt_line = options.begin();
      opt_line != options.end(); ++opt_line){
    for(std::vector<std::string>::const_iterator o_val=opt_line->begin();
	o_val!=opt_line->end(); ++o_val){
      std::cout << *o_val << " ";
    }
    std::cout << std::endl;
    }
  */
  real_cuts CutDefReals;
  category_cuts CutDefCats;
  std::map<std::string,std::string> value_opts;
  get_opts(argv[1],value_opts, CutDefReals, CutDefCats);
  print_cut_table(CutDefReals,CutDefCats);
  cut_container cuts;
  get_opts(argv[1],value_opts,cuts);
  cuts.print_cut_table();

  return 0;
}
