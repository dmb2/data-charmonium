
#include "TLegend.h"
#include "TLatex.h"
#include "TTree.h"
#include "TKey.h"
#include "TCanvas.h"
#include "TH1.h"
#include "TH2D.h"
#include "root-sugar.hh"
#include "TStyle.h"
#include "color.hh"
#include "histo-utils.hh"
#include "unfolding-utils.hh"

#include "RooUnfoldResponse.h"
#include "RooUnfoldBinByBin.h"
#include "RooUnfoldBayes.h"
#include "RooUnfoldSvd.h"

void usage(const char* prog_name){
  MSG("Usage: "<<prog_name<< " -t truth_file.root -i in_file.root -n N -v");
}

int main(const int argc, char* const argv[]){
  char* in_fname=NULL;
  char* truth_fname=NULL;
  int num_iter=4;
  // bool print_validation_plots=false;
  int c;
  while((c = getopt(argc,argv,"n:t:i:")) != -1){
    switch(c){
    // case 'v':
    //   print_validation_plots=true;
    //   break;
    case 'n':
      num_iter=atoi(optarg);
      break;
    case 't':
      truth_fname = optarg;
      break;
    case 'i':
      in_fname = optarg;
      break;
    default:
      abort();
    }
  }
  if(in_fname==NULL || truth_fname==NULL){
    usage(argv[0]);
    exit(1);
  }

  setup_global_style();
  MSG("Input File: "<<in_fname);
  MSG("Truth File: "<<truth_fname);
  TTree* tree = retrieve<TTree>(truth_fname,"mini");
  TFile* reco_file = TFile::Open(in_fname);
  
  const char* variables[] = {"delta_r"//"jet_pt","jet_z","delta_r", 
			     // "jpsi_pt","jpsi_eta"
  };
  int n_evts = tree->GetEntries();
  for(size_t i=0; i < LEN(variables); i++){
    TH1D* reco_hist=nullptr;
    reco_file->GetObject(variables[i],reco_hist);
    if(!reco_hist){
      MSG_ERR("Could not retrieve object named: "<<variables[i]);
      exit(1);
    }
    std::string name(reco_hist->GetName());
    reco_hist->SetName((name+"_reco").c_str());
    TH1D* base_hist = dynamic_cast<TH1D*>(reco_hist->Clone(name.c_str()));
    TH2* response_hist = mc_response(base_hist,tree,n_evts);
    TH1D* unfolded = unfold(response_hist,reco_hist,num_iter,name);
  }
  reco_file->Close();
  
  return 0;
}
