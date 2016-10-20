#include <iostream>
#include <map>
#include <string>
#include "TMath.h"
#include "TMatrixD.h"
#include "TVectorD.h"
#include "TH1D.h"
#include "TH1.h"
#include "TH2D.h"
#include "TTree.h"

#include "histo-meta-data.hh"
#include "histo-utils.hh"
#include "root-sugar.hh"

void usage(const char* name){
  MSG("Usage: "<< name << " -i input.root -o output.root");
}
TMatrixD TH2_to_TMatrix(TH2D* h2){
    int n=h2->GetNbinsX()+2;
    int m=h2->GetNbinsY()+2;
    TMatrixD M(n,m,h2->GetArray(),"D");
    for(int i=0; i < m; i++){
      double norm=0;
      for(int j=0; j < m; j++){
	norm+=M[j][i];
      }
      for(int j=0; j < m; j++){
	if(norm!=0){
	  M[j][i]/=norm;
	}
      }
    }
    return M;
}
TMatrixD make_response_matrix(TTree* tree, TH1* base_hist){
    TH2D* response_hist = setup_response_hist(base_hist);
    response_hist->SetDirectory(NULL);
    response_hist = dynamic_cast<TH2D*>(make_response_hist(response_hist,tree,base_hist->GetName(),"","_response"));
    return TH2_to_TMatrix(response_hist);
}

int main(const int argc, char* const argv[]){
  char* in_fname=nullptr;
  char* out_fname=nullptr;
  int c;
  while((c = getopt(argc,argv,"l:i:o:"))!= -1){
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
  if(in_fname==nullptr ||
     out_fname==nullptr){
    usage(argv[0]);
    exit(1);
  }
  TTree* tree = retrieve<TTree>(in_fname,"mini");
  
  std::map<std::string,TH1D*> HistBook;
  init_hist_book(HistBook);
  TFile out_file(out_fname,"RECREATE");
  for(std::map<std::string,TH1D*>::iterator it=HistBook.begin();
      it!=HistBook.end(); ++it){
    const std::string& name = it->first;
    TMatrixD response_matrix = make_response_matrix(tree,it->second);
    TH1D* hist = dynamic_cast<TH1D*>(make_normal_hist(it->second,tree,("truth_"+name).c_str(),""));
    hist->SetName(("tmp_"+name).c_str());
    hist->SetDirectory(NULL);
    TH1D* truth_hist = dynamic_cast<TH1D*>(hist->Clone(("truth_"+name).c_str()));
    truth_hist->Reset();
    truth_hist->FillRandom(hist,171000);
    TVectorD true_vec(truth_hist->GetNbinsX()+2,truth_hist->GetArray());
    TVectorD rec_vec = response_matrix*true_vec;
    hist->SetName(("truth_"+name).c_str());
    TH1D* rec_hist = dynamic_cast<TH1D*>(hist->Clone(name.c_str()));
    rec_hist->Reset();
    for(int i =0; i < rec_hist->GetNbinsX(); i++){
      rec_hist->SetBinContent(i,rec_vec[i]);
      rec_hist->SetBinError(i,TMath::Sqrt(rec_vec[i]));
    }
  }
  out_file.Write();
  out_file.Close();
    
  return 0;
}
