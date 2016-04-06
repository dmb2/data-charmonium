#include <iostream>
#include "root-sugar.hh"
#include "histo-meta-data.hh"
#include "fit-utils.hh"
#include "histo-utils.hh"
#include "sbs-utils.hh"

#include "TFile.h"
#include "TLegend.h"
#include "TCanvas.h"
#include "TH1D.h"
#include "TTree.h"

#include "RooRealVar.h"
#include "RooAbsPdf.h"
#include "RooFitResult.h"
#include "RooDataSet.h"

void usage(const char* name){
  MSG("Usage: "<<name<<" input.root tree_name lumi");
}

void jpsi_fit(TTree* tree, RooRealVar* mass, RooRealVar* tau,
	      std::map<std::string,sb_info>& sep_var_info, const double lumi){
  RooDataSet data("data","data",RooArgSet(*mass,*tau),RooFit::Import(*tree));
  RooAbsPdf* model = build_model(mass,tau);
  RooFitResult* result = Fit(model,data);
  result->Print();
  print_plot(mass,&data,model,"mass",";J/#psi Mass [GeV]",lumi);
  print_plot(tau,&data,model,"tau",";J/#psi Proper Decay Time [ps]",lumi);

  double mass_width = get_par_val(&result->floatParsFinal(),"sigma_m");
  double mass_mean = get_par_val(&result->floatParsFinal(),"mean_m");
  add_region(mass, "SB", 
	     mass_mean - 13*mass_width,
	     mass_mean -  3*mass_width);
  add_region(mass,"Sig",
	     mass_mean - 3*mass_width,
	     mass_mean + 3*mass_width);
  add_region(mass,"SB",
	     mass_mean + 3*mass_width,
	     mass_mean + 6*mass_width);
  add_region(tau,"Sig", -1,0.25);
  add_region(tau,"SB",0.25,50);
  const double* covmat = result->covarianceMatrix().GetMatrixArray();
  RooAbsPdf* nc_mass_bkg = find_component(model,"NonCoherentMassBkg");
  RooAbsPdf* np_mass_bkg = find_component(model,"NonPromptMassBkg");
  RooAbsPdf* p_mass_bkg = find_component(model,"PromptMassBkg");
  sep_var_info["mass"].regions=mass->getBinningNames();
  //The following function call looks hairy but its just:
  //             Int(NonCoherent + (NonPrompt + Prompt),Sig)
  // sts_ratio = -------------------------------------------
  //             Int(NonCoherent + (NonPrompt + Prompt),SB))
  sep_var_info["mass"].sts_ratio=div(add(get_yield(nc_mass_bkg, mass,"Sig",covmat),
					 add(get_yield(np_mass_bkg, mass,"Sig",covmat),
					     get_yield(p_mass_bkg,mass,"Sig",covmat))),
				     add(get_yield(nc_mass_bkg, mass,"SB",covmat),
					 add(get_yield(np_mass_bkg, mass,"SB",covmat),
					     get_yield(p_mass_bkg,mass,"SB",covmat))));
  RooAbsPdf* tau_sig = find_component(model,"NonPromptTauSig");
  sep_var_info["tau"].regions=tau->getBinningNames();
  sep_var_info["tau"].sts_ratio=div(get_yield(tau_sig,tau,"Sig",covmat),
				    get_yield(tau_sig,tau,"SB",covmat));


}
int main(const int argc, const char* argv[]){
  if(argc !=4){
    usage(argv[0]);
    return 1;
  }
  setup_global_style();
  TFile* file = TFile::Open(argv[1]);
  TTree* tree = retrieve<TTree>(file,argv[2]);
  const double lumi=atof(argv[3]);

  RooRealVar *mass = new RooRealVar("jpsi_m","jpsi_m",JPSIMASS, JPSIMASS-0.4, JPSIMASS+0.5); // stay away from the psi(2S)
  RooRealVar *tau = new RooRealVar("jpsi_tau","Lifetime",-2.,5);
  std::map<std::string,sb_info> sep_var_info;
  jpsi_fit(tree,mass,tau,sep_var_info,lumi);
  MSG_DEBUG(make_cut_expr(mass->getBinningNames(),"Sig") + " && "
	    + make_cut_expr(tau->getBinningNames(),"Sig"));
  for(std::map<std::string,sb_info>::const_iterator it=sep_var_info.begin(); it !=sep_var_info.end(); ++it){
    const std::string& name = it->first;
    const sb_info info = it->second;
    MSG_DEBUG("Var: "<<name<<" Ratio: "<<str_rep(info.sts_ratio)<<" Regions:");
    for(std::list<std::string>::const_iterator r=info.regions.begin(); r!=info.regions.end();++r){
      MSG_DEBUG("\t"<<*r);
    }
  }

  std::map<std::string,TH1D*> HistBook;
  init_hist_book(HistBook);
  const char* variables[] = {"delta_r","jet_pt","jet_eta", "jet_e","jet_z", 
			     "jpsi_pt","jpsi_eta",
			     "tau1","tau2", "tau3","tau21","tau32"};
  const std::string jpsi_sig_region = make_cut_expr(mass->getBinningNames(),"Sig") 
    + " && " + make_cut_expr(tau->getBinningNames(),"Sig");
  char cut_expr[1024];
  snprintf(cut_expr,sizeof(cut_expr)/sizeof(*cut_expr),
	   "(%s)*weight*%.4g*SF",
	   jpsi_sig_region.c_str(),
	   lumi);

  for(size_t i=0; i < LEN(variables); i++){
    TH1* sig_final = print_sbs_stack(tree,HistBook[variables[i]],".pdf",
				     sep_var_info,lumi);
    print_pythia_stack(HistBook[variables[i]],sig_final,lumi,cut_expr,".pdf");
  }

  return 0;
}
