//-*- c++-mode -*-
#pragma once
#include "RooAbsPdf.h"
#include "RooFitResult.h"
  //Particle mass
#define PDGMASS 3.096916

/*
namespace RooFit{
  class RooAbsPdf;
  class RooArgSet;
}
*/
using namespace RooFit;
RooAbsPdf* build_model(RooRealVar* mass, RooRealVar* tau);
RooFitResult* Fit(RooAbsPdf* model);
void print_fit_results(RooAbsPdf* model);
