#include "TROOT.h"
#include "TFile.h"
#include "TTree.h"
#include <iostream>
#include <vector>
using namespace std;
int main(void){
  gROOT->ProcessLine("#include <vector>");
  TFile file("ntuple.root");
  TTree* tree = dynamic_cast<TTree*>(file.Get("JPsi"));
  std::vector<std::vector<int> > *mu_trk_idx=NULL;
  tree->SetBranchAddress("MUONS_index",&mu_trk_idx);
  cout << tree << " "<< tree->GetName() << " "<< tree->GetEntries()<<endl;
  for(int i = 0; i < tree->GetEntries(); i++){
    tree->GetEntry(i);
  }
  return 0;
}
