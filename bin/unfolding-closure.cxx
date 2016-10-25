#include <iostream>
#include <map>
#include <string>
#include "TMath.h"
#include "TMatrixD.h"
#include "TMatrixF.h"
#include "TVectorF.h"
#include "TH1D.h"
#include "TH1.h"
#include "TH2D.h"
#include "TTree.h"
#include "TColor.h"
#include "TCanvas.h"
#include "TLegend.h"

#include "RooUnfoldResponse.h"
#include "RooUnfoldBayes.h"

#include "RooRealVar.h"
#include "RooPolyVar.h"
#include "RooGaussian.h"
#include "RooAddPdf.h"

#include "histo-meta-data.hh"
#include "histo-style.hh"
#include "histo-utils.hh"
#include "root-sugar.hh"

void usage(const char* name){
  MSG("Usage: "<< name << " -i input.root -o output.root");
}
TMatrixD norm_hist_to_mat(TH2* h2){
    int n=h2->GetNbinsX()+2;
    int m=h2->GetNbinsY()+2;
    TMatrixD M(n,m);
    for(int j=0; j < m; j++){
      double norm=0;
      for(int i=0; i < n; i++){
	norm+=h2->GetBinContent(i,j);
      }
      for(int i=0; i < n; i++){
	if(norm!=0){
	  M[i][j]=h2->GetBinContent(i,j)/norm;
	}
      }
    }
    return M;
}

TH1D* fold_truth(TH1D* truth, const TMatrixD& response_matrix, int n_events){
  const std::string name(truth->GetName());
  TH1D* truth_hist = dynamic_cast<TH1D*>(truth->Clone(("truth_"+name).c_str()));
  truth_hist->Reset();
  truth_hist->FillRandom(truth,n_events);
  TVectorD true_vec(truth_hist->GetNbinsX()+2,truth_hist->GetArray());
  TVectorD rec_vec = response_matrix*true_vec;
  TH1D* rec_hist = dynamic_cast<TH1D*>(truth->Clone(name.c_str()));
  rec_hist->Reset();
  for(int i =0; i < rec_hist->GetNbinsX(); i++){
    rec_hist->SetBinContent(i,rec_vec[i]);
    rec_hist->SetBinError(i,TMath::Sqrt(rec_vec[i]));
  }
  return rec_hist;
}
void print_closure_plot(TH2* response_hist, TH1D* truth, TH1D* reco, TH1D* unfolded, const std::string& suffix){
  aesthetic truth_aes=hist_aes("Truth",TColor::GetColor(8,81,156),0,kSolid);
  aesthetic reco_aes=hist_aes("Reconstructed",TColor::GetColor(165,15,21),0,kSolid);
  aesthetic unfolded_aes=hist_aes("Unfolded",TColor::GetColor(0,0,0),0,kSolid);
  style_hist(truth,truth_aes);
  style_hist(reco,reco_aes);
  style_hist(unfolded,unfolded_aes);
  
  TCanvas canv("canv","Canv",1200,600);
  TLegend* leg = init_legend();
  add_to_legend(leg,truth,truth_aes);
  add_to_legend(leg,reco,reco_aes);
  add_to_legend(leg,unfolded,unfolded_aes);
  canv.Divide(2,1);
  canv.cd(1);
  response_hist->Draw("colz");
  canv.cd(2);
  truth->Draw("H");
  reco->Draw("H same");
  unfolded->Draw("H same");
  leg->Draw("lp");
  canv.cd();
  canv.SaveAs((std::string("unfolding_closure_")+truth->GetName()+suffix+".pdf").c_str());
}
void unfold_toy(TH2* response_hist, TH1D* reco, TH1D* truth, int n_itr,
		const std::string& suffix){
    RooUnfoldResponse response(NULL, NULL,response_hist,
			       (std::string(reco->GetName())+"_unfolded").c_str(),reco->GetTitle());
    RooUnfoldBayes unfold(&response,reco,n_itr);
    TH1D* unfolded = dynamic_cast<TH1D*>(unfold.Hreco(RooUnfold::kCovariance));
    print_closure_plot(response_hist,truth,reco,unfolded,suffix);
}
TH2F* linear_response_toy(double x_min,double x_max){
  RooRealVar x("x","x",x_min,x_max);
  RooRealVar y("y","y",x_min,x_max);
  
  RooRealVar a0("a0","a0",x_min,-5,5);
  RooRealVar a1("a1","a1",1,0.5,2);
  //a0 + a1*y
  RooPolyVar fy("fy","fy",y,RooArgSet(a0,a1));
  RooRealVar sigma("sigma","width of gaussian",0.05*(x_max-x_min));
  RooGaussian model("model","Gaussian with shifting mean",x,fy,sigma);
  return dynamic_cast<TH2F*>(model.createHistogram("x,y"));
}
TH2F* quad_response_toy(double x_min,double x_max){
  RooRealVar x("x","x",x_min,x_max);
  RooRealVar y("y","y",x_min,x_max);
  double delta_x = x_max - x_min;
  RooRealVar a0("a0","a0",x_min*x_min/delta_x+x_min,-1e6,1e6);
  RooRealVar a1("a1","a1",2*x_min/delta_x,-1e6,1e6);
  RooRealVar a2("a2","a2",1/delta_x,-1e6,1e6);
  RooRealVar sigma("sigma","width of gaussian",0.05*delta_x);
  // a0 + a1*y + a2*y^2
  RooPolyVar fy("fy2","fy2",y,RooArgSet(a0,a1,a2));
  RooGaussian model("quad_model","Gaussian with shifting mean",x,fy,sigma);
  return dynamic_cast<TH2F*>(model.createHistogram("y,x"));
}
TH2F* width_response_toy(double x_min,double x_max){
  RooRealVar x("x","x",x_min,x_max);
  RooRealVar y("y","y",x_min,x_max);
  RooRealVar a0("a0","a0",0,-5,5);
  RooRealVar a1("a1","a1",1,0.5,2);
  // a0 + a1*y
  RooPolyVar fy("fy","fy",y,RooArgSet(a0,a1));
  RooRealVar b0("b0","b0",0,-5,5);
  RooRealVar b1("b1","b1",0.1,0.5,2);
  // b0 + b1*y
  RooPolyVar fw("fw","fw",y,RooArgSet(b0,b1));
  RooGaussian model("model","Gaussian with shifting width",x,fy,fw);
  return dynamic_cast<TH2F*>(model.createHistogram("x,y"));
}

int main(const int argc, char* const argv[]){
  setup_global_style();
  char* in_fname=nullptr;
  // char* out_fname=nullptr;
  int c;
  while((c = getopt(argc,argv,"l:i:o:"))!= -1){
    switch(c){
    case 'i':
      in_fname=optarg;
      break;
    // case 'o':
    //   out_fname=optarg;
    //   break;
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
  // TTree* tree = retrieve<TTree>(in_fname,"mini");
  
  std::map<std::string,TH1D*> HistBook;
  init_hist_book(HistBook);
  // TFile out_file(out_fname,"RECREATE");
  
  const char* variables[] = {"jet_pt","jet_z","delta_r", 
			     "jpsi_pt","jpsi_eta"};
  size_t n_itr=4;
  for(size_t i=0; i < LEN(variables); i++){
    const std::string name(variables[i]);
    TH1D* base_hist = HistBook[name];
    double x_min(base_hist->GetXaxis()->GetXmin());
    double x_max(base_hist->GetXaxis()->GetXmax());
      
    // MC Response
    TH2D* response_hist = setup_response_hist(base_hist);
    response_hist = dynamic_cast<TH2D*>(make_response_hist(response_hist,tree,base_hist->GetName(),"","_response"));
    
    TH2F* linear_response_hist=linear_response_toy(x_min,x_max);
    linear_response_hist->SetName((name+"_linear_response_hist").c_str());
    TH1D* truth = dynamic_cast<TH1D*>(make_normal_hist(base_hist,tree,"truth_"+name,"","_tmp"));
    TH1D* linear_reco = fold_truth(truth,norm_hist_to_mat(linear_response_hist),171000);
    
    unfold_toy(linear_response_hist,truth,linear_reco,n_itr,"_linear");
  }
  file->Close();
  return 0;
}
