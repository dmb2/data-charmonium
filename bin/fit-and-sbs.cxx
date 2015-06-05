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

#include "AtlasStyle.hh"

void usage(const char* name){
  MSG("Usage: "<<name<<" input.root tree_name lumi");
}

void jpsi_fit(TTree* tree, RooRealVar* mass, RooRealVar* tau,
	      std::map<std::string,sb_info>& sep_var_info, const double lumi){
  RooDataSet data("data","data",RooArgSet(*mass,*tau),RooFit::Import(*tree));
  RooAbsPdf* model = build_model(mass,tau);
  RooFitResult* result = Fit(model,data);
  print_plot(mass,&data,model,"mass",";J/#psi Mass [GeV]",lumi);
  print_plot(tau,&data,model,"tau",";J/#psi Proper Decay Time [ps]",lumi);
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
  result->Print();
}
void psi_fit(TTree* tree,RooRealVar* mass, RooRealVar* tau,
	     std::map<std::string,sb_info>& sep_var_info,
	     const double lumi){
  RooRealVar *psi_m = new RooRealVar("psi_m","psi_m",PSIMASS,PSIMASS-0.25,PSIMASS+0.25);
  RooDataSet data("psi_data","jpsi_data",RooArgSet(*mass,*tau,*psi_m),RooFit::Import(*tree));
  const std::string jpsi_sig_expr = make_cut_expr(mass->getBinningNames(),"Sig") + " && "
    + make_cut_expr(tau->getBinningNames(),"Sig");
  data.reduce(jpsi_sig_expr.c_str());
  RooAbsPdf* model = build_psi_model(psi_m);
  RooFitResult* fit_result = Fit(model,data);
  print_plot(psi_m,&data,model,"psi_m",";#psi(2S) Mass [GeV]",lumi);
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
  const double* covmat = fit_result->covarianceMatrix().GetMatrixArray();
  sep_var_info["psi_m"].regions=psi_m->getBinningNames();
  sep_var_info["psi_m"].sts_ratio=div(get_yield(bkg,psi_m,"Sig",covmat),get_yield(bkg,psi_m,"SB",covmat));
  fit_result->Print();
}
int main(const int argc, const char* argv[]){
  if(argc !=4){
    usage(argv[0]);
    return 1;
  }
  AtlasStyle style;
  style.SetAtlasStyle();
  TFile* file = TFile::Open(argv[1]);
  TTree* tree = retrieve<TTree>(file,argv[2]);
  const double lumi=atof(argv[3]);

  RooRealVar *mass = new RooRealVar("jpsi_m","jpsi_m",JPSIMASS, JPSIMASS-0.5, JPSIMASS+0.5);
  RooRealVar *tau = new RooRealVar("jpsi_tau","Lifetime",-2.,5);
  //unfortunately the order matters, psi_fit uses the fit result of
  //jpsi_fit to define the jpsi mass window
  std::map<std::string,sb_info> sep_var_info;
  jpsi_fit(tree,mass,tau,sep_var_info,lumi);
  // psi_fit(tree,mass,tau,sep_var_info,lumi);
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
  const char* variables[] = {"jet_pt","jet_eta", "jet_z", "jet_e", 
			     "jpsi_pt","jpsi_eta",
			     /*"tau1","tau2", "tau3","tau21","tau32"*/};
  const std::string jpsi_sig_region = make_cut_expr(mass->getBinningNames(),"Sig") 
    + " && " + make_cut_expr(tau->getBinningNames(),"Sig");
  char cut_expr[1024];
  snprintf(cut_expr,sizeof(cut_expr)/sizeof(*cut_expr),
	   "(%s)*weight*%.4g",
	   jpsi_sig_region.c_str(),
	   lumi);
  for(size_t i=0; i < LEN(variables); i++){
    TH1* sig_final = print_sbs_stack(tree,HistBook[variables[i]],".pdf",
				     sep_var_info,lumi);
    TCanvas c1("c1","Canv",600,600);
    sig_final->Draw("e0");
    add_atlas_badge(c1,0.2,0.9,lumi,INTERNAL);
    TLegend leg=*init_legend();
    leg.AddEntry(sig_final,"Period A Data");
    leg.Draw();
    c1.SaveAs((std::string(variables[i])+"_sbs_sub.pdf").c_str());
    print_pythia_stack(HistBook[variables[i]],sig_final,lumi,cut_expr,".pdf");
  }
  return 0;
}
