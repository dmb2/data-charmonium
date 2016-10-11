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
  MSG("Usage: "<< name << "-l lumi -i input.root -o output.root");
}
TMatrixD make_response_matrix(TTree* tree, TH1* base_hist,const char* weight_expr){
    TH2D* response_hist = setup_response_hist(base_hist);
    response_hist->SetDirectory(NULL);
    response_hist = dynamic_cast<TH2D*>(make_response_hist(response_hist,tree,base_hist->GetName(),weight_expr,"_response"));
    for(int i =1; i <= response_hist->GetNbinsX(); i++){
      TH1* hist = response_hist->ProjectionY("_py",i,i+1,"e");
      hist->SetDirectory(NULL);
      double norm_factor = hist->Integral();
      MSG_DEBUG("Norm factor: "<<norm_factor);
      if(norm_factor==0){
	continue;
      }
      for(int j = 0; j < response_hist->GetNbinsY(); j++){
	double bc=response_hist->GetBinContent(i,j);
	response_hist->SetBinContent(i,j,bc/norm_factor);
      }
    }
    return TMatrixD(response_hist->GetNbinsX()+2,response_hist->GetNbinsY()+2,
		    response_hist->GetArray(),"D");
}

int main(const int argc, char* const argv[]){
  char* in_fname=nullptr;
  char* out_fname=nullptr;
  double lumi;
  int c;
  while((c = getopt(argc,argv,"l:i:o:"))!= -1){
    switch(c){
    case 'i':
      in_fname=optarg;
      break;
    case 'o':
      out_fname=optarg;
      break;
    case 'l':
      lumi = atof(optarg);
      break;
    default:
      abort();
    }
  }
  if(in_fname==nullptr ||
     out_fname==nullptr ||
     !std::isfinite(lumi)){
    usage(argv[0]);
    exit(1);
  }
  TTree* tree = retrieve<TTree>(in_fname,"mini");
  
  std::map<std::string,TH1D*> HistBook;
  init_hist_book(HistBook);
  char weight_expr[20];
  snprintf(weight_expr,20,"weight*%.4g*SF",lumi);
  TFile out_file(out_fname,"RECREATE");
  for(std::map<std::string,TH1D*>::iterator it=HistBook.begin();
      it!=HistBook.end(); ++it){
    const std::string& name = it->first;
    TMatrixD response_matrix = make_response_matrix(tree,it->second,"");
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
