//-*- c++-mode -*-
#pragma once
  //Particle mass
#define PDGMASS 3.096916

class RooAbsPdf;
class RooFitResult;
class RooDataSet;
class RooRealVar;
class RooAbsCollection;

RooAbsPdf* build_model(RooRealVar* mass, RooRealVar* tau);
RooFitResult* Fit(RooAbsPdf* model,RooDataSet& data);
void print_fit_results(RooAbsPdf* model, RooDataSet* data, 
		       RooRealVar* mass, RooRealVar *tau);
double get_par_val(const RooAbsCollection* pars,const char* name);

