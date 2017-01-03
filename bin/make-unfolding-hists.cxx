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
  char* out_fname = nullptr;

  int c;
  while((c = getopt(argc,argv,"o:n:i:"))!= -1){
    switch(c){
    case 'i':
      in_fname=optarg;
      break;
    case 'o':
      out_fname=optarg;
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
  
  const char* variables[] = {"delta_r","jet_pt","jet_z",
			     "jpsi_pt","jpsi_eta"
  };
  TFile out_file(out_fname,"RECREATE");
  for(size_t i=0; i < LEN(variables); i++){
    TH1D* base_hist = HistBook[variables[i]];
    TH1* hist = make_normal_hist(base_hist,tree,base_hist->GetName(),"","");
    hist->SetName(base_hist->GetName());
    hist->Write();
  }
  out_file.Close();
  file->Close();
  return 0;
}
