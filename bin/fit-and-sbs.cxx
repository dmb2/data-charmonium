#include <iostream>
#include "root-sugar.hh"
#include "fit-utils.hh"

#include "RooRealVar.h"
#include "RooAbsPdf.h"
#include "RooFitResult.h"
using namespace RooFit;

int main(const int argc, const char* argv[]){
  
  RooRealVar *mass = new RooRealVar("jpsi_m","jpsi_m",PDGMASS, PDGMASS-0.110, PDGMASS+0.110); //Range set to +/- 0.11 GeV of particle mass
  RooRealVar *tau = new RooRealVar("jpsi_lxy","Lifetime",-2.,10.);

  RooAbsPdf* model = build_model(mass,tau);
  RooFitResult* result = Fit(model);
  print_fit_results(model);
  
  return 0;
}
