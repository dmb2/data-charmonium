// System
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <vector>
#include <string>
#include <map>
// Local Includes
#include "cut-flow-studies.hh"
// Root Includes
#include "TFile.h"
#include "TTree.h"
int main(const int argc, const char* argv[]){
  //Parse command line arguments
  std::vector<std::string> arg_list;
  for(int i=0; i < argc; i++){
    arg_list.push_back(std::string(argv[i]));
  }
  TFile* file = new TFile("ntuple.root");
  TTree* tree=static_cast<TTree*>(file->Get("AUX"));
  if(!tree){
    printf("ERROR: could not retrieve tree: %s","AUX");
    return 1;
  }
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
  CutDefCats["NumCharmJet"]=cut<int>(1);
  
  const char* tree_names[] = {"JET","JPSI","MUONS",
			    "PRIVX","SEL_TRACKS",
			    "TRIG","TRUTH_JET"};

  for(size_t i=0; i < sizeof(tree_names)/sizeof(*(tree_names)); i++){
    printf("Adding %s to tree named %s\n",tree_names[i],tree->GetName());
    tree->AddFriend(tree_names[i]);
  }

  process_tree(tree,CutDefReals,CutDefCats);
  file->Close();
  if(file) delete file;
  if(tree) delete tree;
  return 0;
}
