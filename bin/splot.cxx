#include <iostream>
#include "root-sugar.hh"
#include "histo-meta-data.hh"
#include "fit-utils.hh"
#include "tree-utils.hh"
#include "sbs-utils.hh"
#include "histo-utils.hh"
#include "color.hh"

#include "TH1D.h"
#include "TFile.h"
#include "TTree.h"
#include "TStyle.h"
#include "TKey.h"

#include "RooRealVar.h"
#include "RooAbsPdf.h"
#include "RooFitResult.h"
#include "RooDataSet.h"
#include "RooWorkspace.h"



void usage(const char* name){
  MSG("Usage: "<<name<<" -i input.root -t tree_name -l lumi -r fitresult.root -o out.root");
}

int main(const int argc, char* const argv[]){
  char* inFName=NULL;
  char* tree_name = NULL;
  char* fit_fname = NULL;
  char* out_fname = NULL;
  int c;
  double lumi;
  bool print_validation_plots=false;
  
  while((c = getopt(argc,argv,"vo:i:l:t:r:"))!= -1){
    switch(c){
    case 'v':
      print_validation_plots=true;
      break;
    case 'o':
      out_fname=optarg;
      break;
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
  if(fit_fname==NULL || inFName==NULL || tree_name==NULL || !std::isfinite(lumi) ||
     out_fname==NULL){
    usage(argv[0]);
    exit(1);
  }
  setup_global_style();

  TFile* file = TFile::Open(inFName);
  TFile* fit_file = TFile::Open(fit_fname);
  TTree* tree = retrieve<TTree>(file,tree_name);

  RooWorkspace& wkspc = *retrieve<RooWorkspace>(fit_file,"workspace");
  std::map<std::string,RooWorkspace*> syst_wkspaces;
  TIter iter(fit_file->GetListOfKeys());
  TKey *key=NULL;
  while((key=(TKey*)iter())){
    const std::string obj_name(key->GetName());
    if(obj_name.find("workspace")!=std::string::npos &&
       obj_name!="workspace"){
      MSG_DEBUG("Retrieving workspace: "<<obj_name);
      syst_wkspaces[obj_name]=dynamic_cast<RooWorkspace*>(key->ReadObj());
    }
  }

  RooRealVar *mass = wkspc.var("jpsi_m"); 
  RooRealVar *tau = wkspc.var("jpsi_tau");
  RooFitResult* result = dynamic_cast<RooFitResult*>(wkspc.obj("result"));
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
  char cut_expr[1024];
  char data_cut_expr[1024];
  snprintf(cut_expr,sizeof(cut_expr)/sizeof(*cut_expr),
	   "((jpsi_m > %g && jpsi_m < %g) && (jpsi_tau > %g && jpsi_tau < %g))*weight*%.4g*SF*pileup_weight",
	   mass->getMin(),mass->getMax(),
	   tau->getMin(),tau->getMax(),
	   lumi);
  snprintf(data_cut_expr,sizeof(data_cut_expr)/sizeof(*data_cut_expr),
	   "(jpsi_m > %g && jpsi_m < %g) && (jpsi_tau > %g && jpsi_tau < %g)",
	   mass->getMin(),mass->getMax(),
	   tau->getMin(),tau->getMax());
  std::map<std::string,TH1D*> HistBook;
  init_hist_book(HistBook);
  const char* var_names[] = {//"delta_r","jet_z" ,
  			     // "tau1","tau2","tau3","tau21","tau32",
			     "mu1_pt", "mu2_pt",
			     "mu1_eta", "mu2_eta",
			     "vtx_ntrk","vtx_chi2",
			     "vtx_n"//,"jpsi_pt","jpsi_eta",
			     // "jet_pt","jet_eta","pileup"
  };
  std::vector<std::string> variables(var_names,var_names+LEN(var_names));
  // std::vector<std::string> variables = map_keys(HistBook);
  std::vector<TH1*> to_write;
  for(std::vector<std::string>::const_iterator itr=variables.begin();
      itr!=variables.end(); ++itr){
    const std::string var(*itr);
    TH1* base_hist = HistBook[var];
    std::pair<TH1*,TH1*> final_hists=make_splot(tree,base_hist,&wkspc);
    TH1* sig_final = final_hists.first;
    TH1* bkg_final = final_hists.second;
    TH1* bkg_tot_err = dynamic_cast<TH1*>(base_hist->Clone((base_hist->GetName()+std::string("_bkg_tot_err")).c_str()));
    TH1* sig_tot_err = dynamic_cast<TH1*>(base_hist->Clone((base_hist->GetName()+std::string("_sig_tot_err")).c_str()));
    // loop over workspaces, splot them, add in quadrature
    for(std::map<std::string,RooWorkspace*>::iterator it=syst_wkspaces.begin();
	it!=syst_wkspaces.end(); ++it){
      const std::string& syst_name = it->first;
      RooWorkspace* syst_w = it->second;
      std::pair<TH1*,TH1*> syst_var_hists = make_splot(tree,base_hist,syst_w);
      TH1* sig_syst_hist = syst_var_hists.first;
      TH1* bkg_syst_hist = syst_var_hists.second;
      sig_syst_hist->Add(sig_final,-1);
      bkg_syst_hist->Add(bkg_final,-1);
      add_err(bkg_tot_err,bkg_syst_hist);
      add_err(sig_tot_err,sig_syst_hist);
    }
    add_err(sig_final,sig_tot_err);
    add_err(bkg_final,bkg_tot_err);
    if(print_validation_plots){
      print_corr_plot(base_hist,"jpsi_tau",
      		      HistBook["jpsi_tau"]->GetNbinsX(),
      		      tau->getMin(),tau->getMax(),
      		      tree,"_tau_corr.pdf",lumi,cut_expr);
      print_corr_plot(base_hist,"jpsi_m",HistBook["jpsi_m"]->GetNbinsX(),
      		      mass->getMin(),mass->getMax(),
      		      tree,"_m_corr.pdf",lumi,cut_expr);
      print_bkg_splot(tree,(TH1*)bkg_final->Clone(),".pdf",lumi,&wkspc);
    }
    print_splot_stack(tree,base_hist,sig_final,bkg_final,data_cut_expr,lumi,".pdf");
    print_pythia_stack(base_hist,sig_final,lumi,cut_expr,".pdf");
    sig_final->SetName(var.c_str());
    to_write.push_back(sig_tot_err);
    to_write.push_back(sig_final);
  }
  TFile out_file(out_fname,"RECREATE");
  for(std::vector<TH1*>::iterator it=to_write.begin(); it!=to_write.end(); ++it){
    (*it)->Write();
  }
  out_file.Write();
  out_file.Close();
  return 0;
}
