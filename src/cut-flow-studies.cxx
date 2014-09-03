//System
#include <iostream>
#include <cstdlib>

//ROOT 
#include "TFile.h"
#include "TLorentzVector.h"
#include "TTree.h"

//Local
#include "cut-flow-studies.hh"


using std::cout;
using std::cerr;
using std::endl;


int process_tree(TTree* tree, real_cuts CutDefReal, 
		 category_cuts CutDefCat){
  CutDefCat["Nominal"].pass();
  long nEntries = tree->GetEntries();
  return 0;
}
