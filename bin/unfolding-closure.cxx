#include <iostream>
#include <iomanip>
#include <map>
#include <string>
#include "TH1D.h"
#include "TH1.h"
#include "TH2D.h"
#include "TTree.h"


#include "histo-meta-data.hh"
#include "histo-utils.hh"
#include "root-sugar.hh"
#include "unfolding-utils.hh"

void usage(const char* name){
  MSG("Usage: "<< name << " -i input.root -o output.root");
}


int main(const int argc, char* const argv[]){
  setup_global_style();
  char* in_fname=nullptr;
  int n_evts = 171000;
  int n_itr = 4;

  int c;
  while((c = getopt(argc,argv,"n:i:"))!= -1){
    switch(c){
    case 'i':
      in_fname=optarg;
      break;
    case 'n':
      n_itr=atoi(optarg);
      break;
    default:
      abort();
    }
  }
  if(in_fname==nullptr){
    usage(argv[0]);
    exit(1);
  }
  
  TFile* file = TFile::Open(in_fname);
  if(!file){
    MSG_ERR("Could not open file: "<<in_fname);
    exit(-1);
  }
  TTree* tree = nullptr;
  file->GetObject("mini",tree);
  if(!tree){
    MSG_ERR("Could not retrieve tree: mini");
    exit(-2);
  }
  
  std::map<std::string,TH1D*> HistBook;
  init_hist_book(HistBook);
  
  const char* variables[] = {"jet_z","delta_r"//,"jet_pt",
			     // "jpsi_pt","jpsi_eta"
  };
  for(size_t i=0; i < LEN(variables); i++){
    TH1D* base_hist = HistBook[variables[i]];
    // MC Response
    unfold_toy(mc_response,mc_truth,base_hist,tree,n_itr,n_evts,"_mc");
    //Linear response 
    // unfold_toy(linear_response_toy,mc_truth,base_hist,tree,n_itr,n_evts,"_linear");
    // Quadratice response
    // unfold_toy(quad_response_toy,mc_truth,base_hist,tree,n_itr,n_evts,"_quad");
    
    // Custom truth
    unfold_toy(mc_response,gauss_truth,base_hist,tree,n_itr,n_evts,"_gauss_truth");
    unfold_toy(mc_response,dbl_gauss_truth,base_hist,tree,n_itr,n_evts,"_dbl_gauss_truth");
    
    
  }
  file->Close();
  return 0;
}
