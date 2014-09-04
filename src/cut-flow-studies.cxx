//System
#include <iostream>
#include <cstdlib>

//ROOT 
#include "TFile.h"
#include "TLorentzVector.h"
#include "TTree.h"

//Local
#include "cut-flow-studies.hh"
#include "Units.hh"

using namespace Units;
using std::cout;
using std::cerr;
using std::endl;


int process_tree(TTree* tree, real_cuts CutDefReal, 
		 category_cuts CutDefCat){

  CutDefCat["Nominal"].pass();
  tree->SetCacheSize(10*MB);
  tree->SetCacheLearnEntries(5);
  Long64_t nEntries = tree->GetEntries();
  
  for(Long64_t i=0; i < nEntries; i++){
    
  }
  return 0;
}
