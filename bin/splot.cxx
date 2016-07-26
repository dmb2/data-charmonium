#include <iostream>
#include "root-sugar.hh"
#include "histo-meta-data.hh"
#include "fit-utils.hh"
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
  MSG("Usage: "<<name<<" -i input.root -t tree_name -l lumi -r fitresult.root");
}

int main(const int argc, char* const argv[]){
  char* inFName=NULL;
  char* tree_name = NULL;
  char* fit_fname = NULL;
  int c;
  double lumi;
  bool print_validation_plots=true;
  
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

  double stops[]={0.0,0.25,0.5,0.75,1.0};
  heat_gradient(gStyle,stops,LEN(stops));
  
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
  const std::string jpsi_sig_region = make_cut_expr(mass->getBinningNames(),"Sig") 
    + " && " + make_cut_expr(tau->getBinningNames(),"Sig");
  char cut_expr[1024];
  snprintf(cut_expr,sizeof(cut_expr)/sizeof(*cut_expr),
	   "(%s)*weight*%.4g*SF",
	   jpsi_sig_region.c_str(),
	   lumi);
  std::map<std::string,TH1D*> HistBook;
  init_hist_book(HistBook);
  const char* variables[] = {"delta_r","jet_pt","jet_eta", "jet_e",
			     "jet_z" ,
			     "jpsi_pt","jpsi_eta",
			     "tau1","tau2", "tau3","tau21","tau32"
  };
  for(size_t i=0; i < LEN(variables); i++){
    TH1* base_hist = HistBook[variables[i]];
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
      std::pair<TH1*,TH1*> syst_var_hists = make_splot(tree,HistBook[variables[i]],syst_w);
      TH1* sig_syst_hist = syst_var_hists.first;
      TH1* bkg_syst_hist = syst_var_hists.second;
      sig_syst_hist->Add(sig_final,-1);
      sig_syst_hist->Scale(0.5);
      bkg_syst_hist->Add(bkg_final,-1);
      bkg_syst_hist->Scale(0.5);
      add_err(bkg_tot_err,bkg_syst_hist);
      add_err(sig_tot_err,sig_syst_hist);
    }
    add_err(sig_final,sig_tot_err);
    add_err(bkg_final,bkg_tot_err);
    // printf("Hist: %s",sig_tot_err->GetName());
    // for(int i =0 ; i < sig_tot_err->GetNbinsX(); i++){
    //   printf("%g %g %g %g\n",sig_final->GetBinContent(i),
    // 	     sig_final->GetBinError(i),
    // 	     sig_tot_err->GetBinContent(i),
    // 	     sig_tot_err->GetBinError(i));
    // }
    if(print_validation_plots){
      print_corr_plot(HistBook[variables[i]],"jpsi_tau",
		      HistBook["jpsi_tau"]->GetNbinsX(),
		      -3*tau_width,3*tau_width,
		      tree,"_tau_corr.pdf",lumi,cut_expr);
      print_corr_plot(HistBook[variables[i]],"jpsi_m",HistBook["jpsi_m"]->GetNbinsX(),
		      mass_mean-3*mass_width,
		      mass_mean+3*mass_width,
		      tree,"_m_corr.pdf",lumi,cut_expr);
      print_bkg_splot(tree,bkg_final,".pdf",lumi,&wkspc);
    }
    print_splot_stack(tree,HistBook[variables[i]],sig_final,bkg_final,jpsi_sig_region.c_str(),lumi,".pdf");
    print_pythia_stack(HistBook[variables[i]],sig_final,lumi,cut_expr,".pdf");
  }
  return 0;
}
