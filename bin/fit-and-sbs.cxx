#include <iostream>
#include "root-sugar.hh"
#include "fit-utils.hh"

#include "TFile.h"
#include "TTree.h"

#include "RooRealVar.h"
#include "RooAbsPdf.h"
#include "RooFitResult.h"
#include "RooDataSet.h"


void usage(const char* name){
  MSG("Usage: "<<name<<" input.root tree_name");
}

int main(const int argc, const char* argv[]){
  if(argc !=3){
    usage(argv[0]);
    return 1;
  }
  RooRealVar *mass = new RooRealVar("jpsi_m","jpsi_m",PDGMASS, PDGMASS-0.110, PDGMASS+0.110); //Range set to +/- 0.11 GeV of particle mass
  RooRealVar *tau = new RooRealVar("jpsi_lxy","Lifetime",-2.,10.);

  RooAbsPdf* model = build_model(mass,tau);
  TFile* file = TFile::Open(argv[1]);
  TTree* tree = retrieve<TTree>(file,argv[2]);

  RooDataSet data("data","data",RooArgSet(*mass,*tau),RooFit::Import(*tree));

  //RooFitResult* result = Fit(model,data);
  //result->Print();
  print_fit_results(model);
  
  return 0;
}
