// System
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <vector>
#include <string>
#include <map>
// Local Includes
#include "cut-flow-studies.hh"
#include "Units.hh"
// Root Includes
#include "TFile.h"
#include "TTree.h"
using namespace Units;
using std::cout;
using std::endl;
int main(const int argc, const char* argv[]){
  std::vector<std::string> arg_list;
  for(int i=0; i < argc; i++){
    arg_list.push_back(std::string(argv[i]));
  }
  TFile* file = new TFile("ntuple.root");//ntuple.root
  tree_collection Forest; 
  const char* treeNames[] = {"AUX","JET","MU","JPSI",
			    "PRIVX","SEL_TRACKS",
			    "TRIG","TRUTH_JET"};
  for(size_t i=0; i < sizeof(treeNames)/sizeof(*(treeNames)); i++){
    Forest[std::string(treeNames[i])]=dynamic_cast<TTree*>(file->Get(treeNames[i]));
  }

  real_cuts CutDefReals;
  category_cuts CutDefCats;
  CutDefCats["mu_trigger"]=cut<int>(1);
  CutDefCats["nominal"]=cut<int>();
  CutDefCats["num_jets"]=cut<int>(1);
  CutDefReals["jpsi_pt"]=cut<double>(20);
  CutDefReals["jpsi_eta"]=cut<double>(2.5);
  CutDefReals["jet_eta"]=cut<double>(2.5);
  CutDefReals["delta_r"]=cut<double>(0.4);
  CutDefReals["jet_pt"]=cut<double>(45);
  const char* CutNames[]={ "nominal", "mu_trigger","num_jets", "jpsi_pt", "jpsi_eta",
			   "jet_eta","delta_r","jet_pt"}; 
  TFile OutFile("cut_tree.root","RECREATE");
  OutFile.cd();
  TTree OutTree("mini","mini");
  process_tree(Forest,CutDefReals,CutDefCats,OutTree);
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
