// System
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <vector>
#include <string>
#include <map>
// Local Includes
#include "truth-studies.hh"
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
  TFile* file = new TFile("1S0_8.truth.d3pd.root");
  TTree* tree = (TTree*)file->Get("truth");

  real_cuts CutDefReals;
  category_cuts CutDefCats;
  //CutDefCats["mu_trig"]=cut<int>(1);
  CutDefCats["nominal"]=cut<int>();
  CutDefCats["num_jets"]=cut<int>(1);
  CutDefReals["jpsi_pt"]=cut<double>(20);
  CutDefReals["jpsi_eta"]=cut<double>(2.5);
  CutDefReals["jet_eta"]=cut<double>(2.5);
  CutDefReals["delta_r"]=cut<double>(0.4);
  CutDefReals["jet_pt"]=cut<double>(45);
  const char* CutNames[]={ "nominal"/*, "mu_trig"*/,"num_jets", "jpsi_pt", "jpsi_eta",
			   "jet_eta","delta_r","jet_pt"}; 
  TFile OutFile("cut_tree.root","RECREATE");
  OutFile.cd();
  TTree OutTree("mini","mini");
  process_tree(*tree,CutDefReals,CutDefCats,OutTree);
  print_cut_table(CutDefReals,CutDefCats,CutNames,
		  sizeof(CutNames)/sizeof(*CutNames));
  if(tree){ 
    delete tree;
  }
  file->Close();
  if(file){
    delete file;
  }
  OutFile.Write();
  OutFile.Close();
  return 0;
}
