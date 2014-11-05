// System
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <vector>
#include <string>
#include <map>
// Local Includes
#include "truth-studies.hh"
#include "simple-parser.hh"
#include "root-sugar.hh"
#include "Units.hh"
// Root Includes
#include "TFile.h"
#include "TTree.h"

using namespace Units;
using std::cout;
using std::endl;
void usage(const char* name){
  cout <<"Usage: "<< name << " config.conf"<<endl;
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

TTree* tree = retrieve<TTree>(inFName.c_str(),"truth");
  TFile OutFile(outFName.c_str(),"RECREATE");
  OutFile.cd();
  TTree OutTree("mini","mini");

  const char* CutNames[]={ "nominal"/*, "mu_trig"*/,"num_jets", "jpsi_pt", "jpsi_eta",
			   "jet_eta","delta_r","jet_pt"}; 
  process_tree(*tree,CutDefReals,CutDefCats,OutTree);
  print_cut_table(CutDefReals,CutDefCats,CutNames,
		  sizeof(CutNames)/sizeof(*CutNames));
  if(tree){ 
    delete tree;
  }
  OutFile.Write();
  OutFile.Close();
  return 0;
}
