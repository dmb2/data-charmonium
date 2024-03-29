//-*- c++-mode -*-
#pragma once
  //Particle mass
#define JPSIMASS 3.096916
#define PSIMASS 3.686109

class RooAbsPdf;
class RooFitResult;
class RooDataSet;
class RooRealVar;
class RooAbsCollection;

RooAbsPdf* build_model(RooRealVar* mass, RooRealVar* tau, const double n_events,
		       const int mass_n=1, const bool res_single=false,
		       const bool flat_mass=false,
		       const bool flipped=false,
		       const int cb_alpha=10,const int cb_n=1);
RooAbsPdf* build_psi_model(RooRealVar* mass);
RooFitResult* Fit(RooAbsPdf* model,RooDataSet& data);
void print_plot(RooRealVar* var,RooDataSet* data, RooAbsPdf* model, 
		const char* key,const char* title, 
		const double lumi, const char* syst_var=0);
