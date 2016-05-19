#include <iostream>
#include "root-sugar.hh"
#include "histo-meta-data.hh"
#include "fit-utils.hh"
#include "histo-utils.hh"

#include "TFile.h"
#include "TTree.h"

#include "RooRealVar.h"
#include "RooAbsPdf.h"
#include "RooFitResult.h"
#include "RooDataSet.h"
#include "RooWorkspace.h"

void usage(const char* name){
  MSG("Usage: "<<name<<" -i input.root -t tree_name -l lumi -o fitresult.root");
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
  char* outFName=NULL;
  char* tree_name = NULL;
  int c;
  double lumi;
  
  while((c = getopt(argc,argv,"i:l:t:o:"))!= -1){
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
    case 'o':
      outFName=optarg;
      break;
    default:
      abort();
    }
  }
  if(inFName==NULL || tree_name==NULL || !std::isfinite(lumi) ){
    usage(argv[0]);
    exit(1);
  }
  if(outFName==NULL){
    //waste not want not
    outFName=(char*)calloc(16,sizeof(char));
    strncpy(outFName,"fitresult.root",16);
  }
  
  setup_global_style();

  TFile* file = TFile::Open(inFName);
  TTree* tree = retrieve<TTree>(file,tree_name);
  MSG_DEBUG("Fit result is stored in: "<<outFName);
  
  RooRealVar *mass = new RooRealVar("jpsi_m","jpsi_m",JPSIMASS, JPSIMASS-0.4, JPSIMASS+0.5); // stay away from the psi(2S)
  RooRealVar *tau = new RooRealVar("jpsi_tau","Lifetime",-2.,5);
  //TFile out_file(outFName,"RECREATE");
  RooWorkspace w("workspace","Workspace for Fit");
  jpsi_fit(tree,mass,tau,lumi,w);
  //out_file.Write();
  //out_file.Close();
  w.Print();
  w.writeToFile(outFName);
  
  return 0;
}
