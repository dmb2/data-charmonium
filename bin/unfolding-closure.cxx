#include <iostream>
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
  int c;
  while((c = getopt(argc,argv,"i:"))!= -1){
    switch(c){
    case 'i':
      in_fname=optarg;
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
  
  const char* variables[] = {"jet_z"//"jet_pt","jet_z","delta_r", 
			     //		     "jpsi_pt","jpsi_eta"
  };
  size_t n_itr=4;
  for(size_t i=0; i < LEN(variables); i++){
    const std::string name(variables[i]);
    TH1D* base_hist = HistBook[name];
      
    // MC Response
    // TH2D* response_hist = setup_response_hist(base_hist);
    // response_hist = dynamic_cast<TH2D*>(make_response_hist(response_hist,tree,base_hist->GetName(),"","_response"));
    
    TH2F* linear_response_hist=linear_response_toy(base_hist,171000);
    for(int bi=0; bi < linear_response_hist->GetNbinsX(); bi++){
      for(int bj=0; bj < linear_response_hist->GetNbinsY(); bj++){
	std::cout<<linear_response_hist->GetBinContent(bi,bj)<<", ";
      }
      std::cout<<std::endl;
    }
    linear_response_hist->SetName((name+"_linear_response_hist").c_str());
    TH1D* truth = dynamic_cast<TH1D*>(make_normal_hist(base_hist,tree,"truth_"+name));
    truth->SetName(name.c_str());
    TH1D* linear_reco = fold_truth(truth,norm_hist_to_mat(linear_response_hist),171000);
    unfold_toy(linear_response_hist,linear_reco,truth,n_itr,"_linear");
  }
  file->Close();
  return 0;
}
