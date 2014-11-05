#include <iostream>
#include <sstream>
#include <cctype>
#include <algorithm>
#include <vector>

#include "simple-parser.hh"


std::string strip_whitespace(const std::string& inString){
  std::string outString(inString); 
  outString.erase(std::remove_if(outString.begin(),outString.end() ,isspace),outString.end());
  return outString;
}
std::string strip_comment(const std::string& inString){
  std::string outString(inString);
  size_t cmnt_pos=0;
  cmnt_pos=outString.find("#");
  if(cmnt_pos!=std::string::npos){
    outString.erase(cmnt_pos);
  }
  return outString;
}
int parse_line(const std::string& line, std::vector<std::string>& options,const size_t line_n){
  /// This function takes the line and looks for two cases: statements
  /// of the form 'a=b' or cut definitions of the form 'foo ??
  /// val,cut_type' where ?? is a comparison operator, val is the cut
  /// value, and cut_type is either "cat" or "real".
  size_t idx=0;
  idx=line.find('=');
  if(idx!=std::string::npos && line[idx+1]!='=' &&
     line[idx-1]!='>' && line[idx-1]!='<' && line[idx-1]!='!'){
    options.push_back(line.substr(0,idx));
    options.push_back(line.substr(idx+1));
    return 0;
  }

  // extract cutname, comparison operator, cut value, and cut type
  const char* comp_op[] = {"==", "!=","<=", ">=", ">", "<"};
  idx=0;
  for(size_t i=0; i < sizeof(comp_op)/sizeof(*comp_op); i++){
    idx=line.find(std::string(comp_op[i]));
    if(idx!=std::string::npos){
      options.push_back(line.substr(0,idx));
      options.push_back(comp_op[i]);
      break;
    }
  }
  if(idx==std::string::npos){
    std::cerr<<"ERROR: Could not find a comparison operator at line "<<line_n <<std::endl;
    return 1;
  }
  //adjust the index position just beyond the comparison operator
  idx+= (line[idx+1]=='=') ? 2 : 1;
  size_t delim_pos;
  delim_pos=line.find(',');
  if(delim_pos!=std::string::npos){
    options.push_back(line.substr(idx,delim_pos-idx));
  }
  else{
    std::cerr<<"ERROR: Could not find a \",\" to separate cut from cut type at line "<<line_n<<std::endl;
  }
  options.push_back(line.substr(delim_pos+1));
  return 0;
}
int parse_file(std::ifstream& file,std::vector<std::vector<std::string > >& options){
  std::string line;
  size_t line_n=0;
  while(std::getline(file,line)){
    line_n++;
    std::vector<std::string> line_opts;
    line_opts.reserve(3);
    line = strip_whitespace(line);
    if(line[0]=='#') {
      continue;
    }
    line = strip_comment(line);
    if(parse_line(line,line_opts,line_n)){
      break;
    };
    options.push_back(line_opts);
  }
  return 0;
}
void get_opts(const char* opt_fname,std::string& inFName,std::string& outFName,
	      real_cuts& CutDefReals, category_cuts& CutDefCats){
  std::cout<<"Using config file: "<<opt_fname<<std::endl;
  std::ifstream file(opt_fname);
  std::vector<std::vector<std::string > > options;
  parse_file(file,options);
  for(std::vector<std::vector<std::string > >::const_iterator opt_line = options.begin(); 
      opt_line!=options.end(); ++opt_line){
    const std::vector<std::string>&  opt = *opt_line;
    if(opt.size()==2){
      if(opt[0]=="inFile"){
	inFName=opt[1];
      }
      else if(opt[0]=="outFile"){
	outFName=opt[1];
      }
    }
    else if(opt.size()==4){
      if(opt[3]=="cat"){
	CutDefCats[opt[0]]=cut<int>(atoi(opt[2].c_str()), opt[1]);
      }
      else if(opt[3]=="real"){
	CutDefReals[opt[0]]=cut<double>(atof(opt[2].c_str()), opt[1]);
      }
    }
  }
}
