#include <cstdlib>
#include <iostream>
#include "cut-flow-studies.hh"
#include <vector>
#include <string>
#include <map>

int main(const int argc, const char* argv[]){
  //Parse command line arguments
  std::vector<std::string> arg_list;
  for(int i=0; i < argc; i++){
    arg_list.push_back(std::string(argv[i]));
    // std::cout<<"Got argument: "<<argv[i]<<std::endl;
  }
  TTree* tree=NULL;
  real_cuts CutDefReals;
  category_cuts CutDefCats;
  CutDefCats["Nominal"]=cut<int>();
  CutDefCats["NumMuons"]=cut<int>(2);
  CutDefCats["JPsiCand"]=cut<int>(1);
  CutDefCats["NumJets"]=cut<int>(1);
  CutDefReals["JPsiPt"]=cut<double>(20*1e6);
  CutDefReals["JetPt"]=cut<double>(45*1e6);
  CutDefReals["JPsiEta"]=cut<double>(2.5);
  CutDefReals["JetEta"]=cut<double>(2.5);
  CutDefCats["NumCharmJet"]cut<int>(1);
  
  
  process_tree(tree,CutDefReals,CutDefCats);
  
  return 0;
}
