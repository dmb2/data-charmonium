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
num_err process_psi(RooRealVar* psi_m, RooDataSet& data){
  // RooDataSet data("data","data",RooArgSet(*psi_m),RooFit::Import(tree));
  // data.reduce(jpsi_sig_expr.c_str());
  RooAbsPdf* model = build_psi_model(psi_m);
  RooFitResult* fit_result = Fit(model,data);
  print_plot(psi_m,&data,model,"psi_m",";#psi(2S) Mass [GeV]");
  fit_result->Print();
  double mass_width = get_par_val(&fit_result->floatParsFinal(),"psi_m_sigma");
  double mass_mean = get_par_val(&fit_result->floatParsFinal(),"psi_m_mean");
  add_region(psi_m, "SB", 
	     mass_mean - 10*mass_width,
	     mass_mean -  3*mass_width);
  add_region(psi_m,"Sig",
	     mass_mean - 3*mass_width,
	     mass_mean + 3*mass_width);
  add_region(psi_m,"SB",
	     mass_mean + 3*mass_width,
	     mass_mean + 10*mass_width);
  RooAbsPdf* bkg = find_component(model,"Background");
  return div(get_yield(bkg,psi_m,"Sig"),get_yield(bkg,psi_m,"SB"));
}
int main(const int argc, const char* argv[]){
  if(argc !=3){
    usage(argv[0]);
    return 1;
  }
  AtlasStyle style;
  style.SetAtlasStyle();

  RooRealVar *psi_m = new RooRealVar("psi_m","psi_m",PSIMASS,PSIMASS-0.25,PSIMASS+0.25);
  RooRealVar *mass = new RooRealVar("jpsi_m","jpsi_m",JPSIMASS, JPSIMASS-0.5, JPSIMASS+0.5);
  RooRealVar *tau = new RooRealVar("jpsi_tau","Lifetime",-2.,5);

  RooAbsPdf* model = build_model(mass,tau);
  TFile* file = TFile::Open(argv[1]);
  TTree* tree = retrieve<TTree>(file,argv[2]);

  RooDataSet data("data","data",RooArgSet(*mass,*tau),RooFit::Import(*tree));
  RooFitResult* result = Fit(model,data);
  print_plot(mass,&data,model,"mass",";J/#psi Mass [GeV]");
  print_plot(tau,&data,model,"tau",";J/#psi Proper Decay Time [ps]");
  double mass_width = get_par_val(&result->floatParsFinal(),"sigma_m");
  double mass_mean = get_par_val(&result->floatParsFinal(),"mean_m");

  add_region(mass, "SB", 
	     mass_mean - 11*mass_width,
	     mass_mean -  3*mass_width);
  add_region(mass,"Sig",
	     mass_mean - 3*mass_width,
	     mass_mean + 3*mass_width);
  add_region(mass,"SB",
	     mass_mean + 3*mass_width,
	     mass_mean + 8*mass_width);
  add_region(tau,"Sig", -1,0.25);
  add_region(tau,"SB",0.25,50);
  //slightly obnoxious but the RooDataSet destructively reduces itself so we have to make a copy.
  RooDataSet psi_data("psi_data","jpsi_data",RooArgSet(*mass,*tau,*psi_m),RooFit::Import(*tree));
  const std::string jpsi_sig_expr = make_cut_expr(mass->getBinningNames(),"Sig") + " && "
    + make_cut_expr(tau->getBinningNames(),"Sig");
  num_err psi_stsR = process_psi(psi_m, *dynamic_cast<RooDataSet*>(psi_data.reduce(jpsi_sig_expr.c_str())));
  result->Print();

  const char* variables[] = {"jet_pt","jet_z","jet_e",
			   "jpsi_pt","tau1","tau2",
			   "tau3","tau21","tau32"};
  do_sbs(variables,sizeof(variables)/sizeof(*variables),
  	 tree,model,mass,tau,psi_m, psi_stsR, "_sbs.pdf");
  return 0;
}
