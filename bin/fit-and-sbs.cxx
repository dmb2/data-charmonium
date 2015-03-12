#include <iostream>
#include "root-sugar.hh"
#include "fit-utils.hh"
#include "sbs-utils.hh"

#include "TFile.h"
#include "TTree.h"

#include "RooRealVar.h"
#include "RooAbsPdf.h"
#include "RooFitResult.h"
#include "RooDataSet.h"

#include "AtlasStyle.hh"

void usage(const char* name){
  MSG("Usage: "<<name<<" input.root tree_name");
}

int main(const int argc, const char* argv[]){
  if(argc !=3){
    usage(argv[0]);
    return 1;
  }
  AtlasStyle style;
  style.SetAtlasStyle();

  RooRealVar *mass = new RooRealVar("jpsi_m","jpsi_m",PDGMASS, PDGMASS-0.5, PDGMASS+0.5);
  RooRealVar *tau = new RooRealVar("jpsi_tau","Lifetime",-2.,5);

  RooAbsPdf* model = build_model(mass,tau);
  TFile* file = TFile::Open(argv[1]);
  TTree* tree = retrieve<TTree>(file,argv[2]);

  RooDataSet data("data","data",RooArgSet(*mass,*tau),RooFit::Import(*tree));
  RooFitResult* result = Fit(model,data);
  if (result){
    result->Print();
  }
  print_fit_results(model,&data,mass,tau);
  //this sucks
  double mass_width = dynamic_cast<RooRealVar*>(result->floatParsFinal().find("mass_sigma"))->getVal();
  double mass_mean = dynamic_cast<RooRealVar*>(result->floatParsFinal().find("mass_mean"))->getVal();

  const char* variables[] = {"jet_pt","jet_z","jet_e",
			   "jpsi_pt","tau1","tau2",
			   "tau3","tau21","tau32"};
  add_region(mass, "SB", 
	     mass_mean - 11*mass_width,
	     mass_mean -  3*mass_width);
  add_region(mass,"Sig",
	     mass_mean - 3*mass_width,
	     mass_mean + 3*mass_width);
  add_region(mass,"SB",
	     mass_mean + 3*mass_width,
	     mass_mean + 8*mass_width);
  
  do_sbs(variables,sizeof(variables)/sizeof(*variables),
  	 tree,model,mass,"_sbs.pdf");
  return 0;
}
