#include "TFile.h"
#include "TTree.h"
#include <iostream>
using namespace std;
int main(const int argc, const char* argv[]){
  TFile file("ntuple.root");
  TTree* tree = dynamic_cast<TTree*>(file.Get("MU"));
  cout << tree << " "<< tree->GetName() << " "<< tree->GetEntries()<<endl;
  for(int i = 0; i < tree->GetEntries(); i++){
    tree->GetEntry(i);
  }
  return 0;
}
