// System
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <vector>
#include <string>
#include <map>
// Local Includes
#include "simple-parser.hh"
#include "cut-flow-studies.hh"
#include "Units.hh"
// Root Includes
#include "TFile.h"
#include "TTree.h"
using namespace Units;
void usage(const char* prog_name){
  std::cout << "Usage: "<<prog_name<< " config.conf"<<std::endl;
}
int main(const int argc, const char* argv[]){
  if(argc != 2) {
    usage(argv[0]);
    return 1;
  }
  std::string inFName;
  std::string outFName;
  real_cuts CutDefReals;
  category_cuts CutDefCats;
  get_opts(argv[1],inFName,outFName, CutDefReals, CutDefCats);

  TFile* file = new TFile(inFName.c_str());
  tree_collection Forest; 
  const char* treeNames[] = {"AUX","JET","MU","JPSI",
			    "PRIVX","SEL_TRACKS",
			    "TRIG","TRUTH_JET"};
  for(size_t i=0; i < sizeof(treeNames)/sizeof(*(treeNames)); i++){
    Forest[std::string(treeNames[i])]=dynamic_cast<TTree*>(file->Get(treeNames[i]));
  }
  std::cout <<"Opening output file"<<std::endl;
  TFile OutFile(outFName.c_str(),"RECREATE");

  OutFile.cd();
  TTree OutTree("mini","mini");
  process_tree(Forest,CutDefReals,CutDefCats,OutTree);
  //FIXME
  const char* CutNames[]={ "nominal", "trigger","num_jets", "jpsi_pt", "jpsi_eta",
			   "jet_eta","delta_r","jet_pt"}; 
  print_cut_table(CutDefReals,CutDefCats,CutNames,
		  sizeof(CutNames)/sizeof(*CutNames));
  for(tree_collection::iterator it=Forest.begin(); it != Forest.end(); ++it){
    if(it->second) delete it->second;
  }
  file->Close();
  if(file) delete file;
  OutFile.Write();
  OutFile.Close();
  return 0;
}
