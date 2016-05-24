#include <iostream>
#include "root-sugar.hh"
#include "histo-meta-data.hh"
#include "fit-utils.hh"
#include "sbs-utils.hh"
#include "histo-utils.hh"

#include "TH1D.h"
#include "TFile.h"
#include "TTree.h"

#include "RooRealVar.h"
#include "RooAbsPdf.h"
#include "RooFitResult.h"
#include "RooDataSet.h"
#include "RooWorkspace.h"


double get_par_val(const RooAbsCollection* pars,const char* name){
  //this sucks
  return dynamic_cast<RooRealVar*>(pars->find(name))->getVal();
}
void usage(const char* name){
  MSG("Usage: "<<name<<" -i input.root -t tree_name -l lumi");
}

void jpsi_fit(TTree* tree, RooRealVar* mass, RooRealVar* tau,
	      const double lumi, RooWorkspace& wkspc){
  RooDataSet data("data","data",RooArgSet(*mass,*tau),RooFit::Import(*tree));
  RooAbsPdf* model = build_model(mass,tau,data.numEntries());
  RooFitResult* result = Fit(model,data);
  model->SetName("model");
  //model->Write();
  result->SetName("result");
  result->Print();
  //result->Write();
  wkspc.import(data);
  wkspc.import(*model);
  wkspc.import(*result);
  print_plot(mass,&data,model,"mass",";J/#psi Mass [GeV]",lumi);
  print_plot(tau,&data,model,"tau",";J/#psi Proper Decay Time [ps]",lumi);
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

  RooWorkspace& wkspc = *retrieve<RooWorkspace>(fit_file,"workspace");

  RooRealVar *mass = wkspc.var("jpsi_m"); 
  RooRealVar *tau = wkspc.var("jpsi_tau");
  RooFitResult* result = dynamic_cast<RooFitResult*>(wkspc.obj("result"));
  double mass_width = get_par_val(&result->floatParsFinal(),"sigma_m");
  double mass_mean = get_par_val(&result->floatParsFinal(),"mean_m");
  double tau_width = std::max(get_par_val(&result->floatParsFinal(),"sigma_t1"),
			      get_par_val(&result->floatParsFinal(),"sigma_t2"));
  
  add_region(mass,"Sig",
	     mass_mean - 3*mass_width,
	     mass_mean + 3*mass_width);
  add_region(tau,"Sig", -3*tau_width,3*tau_width);
  const std::string jpsi_sig_region = make_cut_expr(mass->getBinningNames(),"Sig") 
    + " && " + make_cut_expr(tau->getBinningNames(),"Sig");
  char cut_expr[1024];
  snprintf(cut_expr,sizeof(cut_expr)/sizeof(*cut_expr),
	   "(%s)*weight*%.4g*SF",
	   jpsi_sig_region.c_str(),
	   lumi);
  std::map<std::string,TH1D*> HistBook;
  init_hist_book(HistBook);
  const char* variables[] = {// "delta_r","jet_pt","jet_eta", "jet_e",
			     "jet_z", 
			     // "jpsi_pt","jpsi_eta",
			     "tau1"// ,"tau2", "tau3","tau21","tau32"
  };
  for(size_t i=0; i < LEN(variables); i++){
    TH1* sig_final=print_splot_stack(tree,HistBook[variables[i]],".pdf",lumi,&wkspc);
    print_pythia_stack(HistBook[variables[i]],sig_final,lumi,cut_expr,".pdf");
  }
  return 0;
}
