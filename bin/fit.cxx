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
  MSG("Usage: "<<name<<" -i input.root -t tree_name -l lumi -o fitresult.root -s");
}
void fit_pdf(RooAbsPdf* pdf,RooDataSet& data,RooRealVar* mass, RooRealVar* tau,
	     const double lumi,const std::string& name, RooWorkspace& wkspc){
  RooFitResult* result = Fit(pdf,data);
  pdf->SetName("model");
  result->SetName("result");
  result->Print();
  wkspc.import(*pdf);
  wkspc.import(*result);
  print_plot(mass,&data,pdf,"mass",";J/#psi Mass [GeV]",lumi,name!=""? name.c_str(): NULL);
  print_plot(tau,&data,pdf,"tau",";J/#psi Proper Decay Time [ps]",lumi,name!=""? name.c_str(): NULL);
}
void jpsi_fit(TTree* tree, RooRealVar* mass, RooRealVar* tau,
	      const double lumi, const char* outFName,bool do_syst){
  RooDataSet data("data","data",RooArgSet(*mass,*tau),RooFit::Import(*tree));
  std::map<std::string,RooAbsPdf*> syst_pdfs;
  TFile* file = TFile::Open(outFName,"UPDATE");
  
  if(do_syst){
    RooWorkspace* syst_ws = NULL;
    syst_pdfs["resolution"]=build_model(mass,tau,data.numEntries(),1,true);
    syst_pdfs["lifetime"]=build_model(mass,tau,data.numEntries(),1,false,false,true);
    // syst_pdfs["mass0"]=build_model(mass,tau,data.numEntries(),0);
    syst_pdfs["mass2"]=build_model(mass,tau,data.numEntries(),2);
    syst_pdfs["mass3"]=build_model(mass,tau,data.numEntries(),3);
    syst_pdfs["mass_exp"]=build_model(mass,tau,data.numEntries(),1,false,true);
    syst_pdfs["crystal_ball_alpha10_n1"]=build_model(mass,tau,data.numEntries(),1,false,false,false,10,1);
    syst_pdfs["crystal_ball_alpha10_n5"]=build_model(mass,tau,data.numEntries(),1,false,false,false,10,5);
    syst_pdfs["crystal_ball_alpha1_n1"]=build_model(mass,tau,data.numEntries(),1,false,false,false,1,1);
    for(std::map<std::string,RooAbsPdf*>::iterator it=syst_pdfs.begin(); it !=syst_pdfs.end(); ++it){
      const std::string& syst_name = it->first;
      RooAbsPdf* syst_pdf = it->second;
      syst_ws = new RooWorkspace(("workspace_"+syst_name).c_str(),("Workspace for syst variation "+syst_name).c_str());
      fit_pdf(syst_pdf,data,mass,tau,lumi,syst_name,*syst_ws);
      syst_ws->Print();
      // syst_ws->writeToFile(outFName,false);
      syst_ws->Write();
    }
  }
  
  RooWorkspace w("workspace","Workspace for Fit");
  RooAbsPdf* model = build_model(mass,tau,data.numEntries());
  fit_pdf(model,data,mass,tau,lumi,"",w);
  w.Print();
  // w.writeToFile(outFName);
  w.Write();
  file->Write();
  file->Close();
}
int main(const int argc, char* const argv[]){
  char* inFName=NULL;
  char* outFName=NULL;
  char* tree_name = NULL;
  int c;
  double lumi;
  bool do_syst = false;
  while((c = getopt(argc,argv,"i:l:t:o:s"))!= -1){
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
    case 's':
      do_syst = true;
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
  jpsi_fit(tree,mass,tau,lumi,outFName,do_syst);
  
  return 0;
}
