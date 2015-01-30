//-*- c++-mode -*-
#pragma once
  //Particle mass
#define PDGMASS 3.096916

class RooAbsPdf;
class RooFitResult;
class RooDataSet;
class RooRealVar;

RooAbsPdf* build_model(RooRealVar* mass, RooRealVar* tau);
RooFitResult* Fit(RooAbsPdf* model,RooDataSet& data);
void print_fit_results(RooAbsPdf* model);
