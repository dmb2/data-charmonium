#include <iostream>
#include "root-sugar.hh"
#include "histo-meta-data.hh"
#include "histo-utils.hh"
#include "sbs-utils.hh"
#include "fit-utils.hh"

#include "TH1D.h"
#include "TFile.h"
#include "TTree.h"

#include "RooRealVar.h"
#include "RooAbsCollection.h"
#include "RooAbsPdf.h"
#include "RooFitResult.h"
#include "RooDataSet.h"
#include "RooWorkspace.h"

void usage(const char* name){
  MSG("Usage: "<<name<<" -i input.root -t tree_name -l lumi -r fitresult.root");
}

double get_par_val(const RooAbsCollection* pars,const char* name){
  //this sucks
  return dynamic_cast<RooRealVar*>(pars->find(name))->getVal();
}

int main(const int argc, char* const argv[]){
  char* inFName=NULL;
  char* tree_name = NULL;
  char* fit_fname = NULL;
  int c;
  double lumi;
  
  while((c = getopt(argc,argv,"i:l:t:r:"))!= -1){
    switch(c){
    case 'i':
      inFName=optarg;
      break;
    case 'l':
      lumi=atof(optarg);
      break;
    case 't':
      tree_name=optarg;
      break;
    case 'r':
      fit_fname=optarg;
      break;
    default:
      abort();
    }
  }
  if(fit_fname==NULL || inFName==NULL || tree_name==NULL || !std::isfinite(lumi) ){
    usage(argv[0]);
    exit(1);
  }
  setup_global_style();

  TFile* file = TFile::Open(inFName);
  TFile* fit_file = TFile::Open(fit_fname);
  TTree* tree = retrieve<TTree>(file,tree_name);

  RooWorkspace* wkspc = retrieve<RooWorkspace>(fit_file,"workspace");
  RooAbsPdf* model = wkspc->pdf("model");
  RooFitResult* result = dynamic_cast<RooFitResult*>(wkspc->obj("result"));
  result->Print();
  RooRealVar *mass = wkspc->var("jpsi_m"); 
  RooRealVar *tau = wkspc->var("jpsi_tau");
  std::map<std::string,sb_info> sep_var_info;
  double mass_width = get_par_val(&result->floatParsFinal(),"sigma_m");
  double mass_mean = get_par_val(&result->floatParsFinal(),"mean_m");
  double tau_width = std::max(get_par_val(&result->floatParsFinal(),"sigma_t1"),
			      get_par_val(&result->floatParsFinal(),"sigma_t2"));
  add_region(mass, "SB", 
	     mass_mean - 13*mass_width,
	     mass_mean -  3*mass_width);
  add_region(mass,"Sig",
	     mass_mean - 3*mass_width,
	     mass_mean + 3*mass_width);
  add_region(mass,"SB",
	     mass_mean + 3*mass_width,
	     mass_mean + 6*mass_width);
  add_region(tau,"Sig", -3*tau_width,3*tau_width);
  add_region(tau,"SB",3*tau_width,50);
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
