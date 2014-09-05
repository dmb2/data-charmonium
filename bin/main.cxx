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
  //Parse command line arguments
  std::vector<std::string> arg_list;
  for(int i=0; i < argc; i++){
    arg_list.push_back(std::string(argv[i]));
  }
  TFile* file = new TFile("ntuple.root");

  tree_collection Forest; 
  const char* treeNames[] = {"AUX","JET","MUONS","JPSI",
			    "PRIVX","SEL_TRACKS",
			    "TRIG","TRUTH_JET"};
  for(size_t i=0; i < sizeof(treeNames)/sizeof(*(treeNames)); i++){
    printf("Added %s to the forest. \n",treeNames[i]);
    Forest[std::string(treeNames[i])]=dynamic_cast<TTree*>(file->Get(treeNames[i]));
  }

  real_cuts CutDefReals;
  category_cuts CutDefCats;
  CutDefCats["Nominal"]=cut<int>();
  CutDefCats["NumMuons"]=cut<int>(2);
  CutDefCats["JPsiCand"]=cut<int>(1);
  CutDefCats["NumJets"]=cut<int>(1);
  CutDefReals["JPsiPt"]=cut<double>(20*GeV);
  CutDefReals["JetPt"]=cut<double>(45*GeV);
  CutDefReals["JPsiEta"]=cut<double>(2.5);
  CutDefReals["JetEta"]=cut<double>(2.5);
  CutDefCats["NumCharmJet"]=cut<int>(1);
  const char* CutNames[]={ "Nominal", "NumMuons", "JPsiCand", 
			   "NumJets", "JPsiPt", "JetPt",
			   "JPsiEta","JetEta"};

  process_tree(Forest,CutDefReals,CutDefCats);

  real_cuts::iterator rCutHandle;
  category_cuts::iterator cCutHandle;
  for(size_t i=0; i < sizeof(CutNames)/sizeof(*CutNames); i++){
    cCutHandle=CutDefCats.find(CutNames[i]);
    if(cCutHandle!=CutDefCats.end()){
      print_cut_summary(cCutHandle->first, cCutHandle->second);
    }
    else{
      rCutHandle=CutDefReals.find(CutNames[i]);
      if(rCutHandle!=CutDefReals.end()){
	print_cut_summary(rCutHandle->first,rCutHandle->second);
      }
    }
  }

  //Chop down the forest...
  for(tree_collection::iterator it=Forest.begin(); it != Forest.end(); ++it){
    if(it->second) delete it->second;
  }
  file->Close();
  if(file) delete file;
  return 0;
}
