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
#include "TCanvas.h"

#include "RooUnfoldResponse.h"
#include "RooUnfoldBayes.h"

#include "RooRealVar.h"
#include "RooPolyVar.h"
#include "RooGaussian.h"
#include "RooAddPdf.h"

#include "histo-meta-data.hh"
#include "histo-utils.hh"
#include "root-sugar.hh"

void usage(const char* name){
  MSG("Usage: "<< name << " -i input.root -o output.root");
}
template<class Hist2D>
TMatrixD norm_hist_to_mat(Hist2D* h2){
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
void print_closure_plot(TH2D* response_hist, TH1D* truth, TH1D* reco, TH1D* unfolded){
  TCanvas canv("canv","Canv",1200,600);
  canv.Divide(2,1);
  canv.cd(1);
  response_hist->Draw("colz");
  canv.cd(2);
  truth->Draw();
  reco->Draw("same");
  unfolded->Draw("same");
  canv.cd();
  canv.SaveAs((std::string("unfolding_closure_")+truth->GetName()+".pdf").c_str());
}
int main(const int argc, char* const argv[]){
  setup_global_style();
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
  
  const char* variables[] = {"jet_pt","jet_z","delta_r", 
			     "jpsi_pt","jpsi_eta"};
  size_t n_itr=4;
  for(size_t i=0; i < LEN(variables); i++){
    const std::string name(variables[i]);
    TH1D* base_hist = HistBook[name];
      
    // MC Response
    TH2D* response_hist = setup_response_hist(base_hist);
    response_hist->SetDirectory(NULL);
    response_hist = dynamic_cast<TH2D*>(make_response_hist(response_hist,tree,base_hist->GetName(),"","_response"));
    TMatrixD response_matrix = norm_hist_to_mat(response_hist);
    RooUnfoldResponse response(NULL, NULL,response_hist,
			       (name+"_unfolded").c_str(),base_hist->GetTitle());
    double x_min(base_hist->GetXaxis()->GetXmin());
    double x_max(base_hist->GetXaxis()->GetXmax());
    
    RooRealVar x("x","x",x_min,x_max);
    RooRealVar y("y","y",x_min,x_max);
    // Toy Response gaussian + linear mean
    RooRealVar a0("a0","a0",0,-5,5);
    RooRealVar a1("a1","a1",1,0.5,2);
    RooPolyVar fy("fy","fy",y,RooArgSet(a0,a1));
    RooRealVar sigma("sigma","width of gaussian",0.5);
    RooGaussian model("model","Gaussian with shifting mean",x,fy,sigma);
    TH2F* linear_response_hist = dynamic_cast<TH2F*>(model.createHistogram("x,y"));
    // Toy Response gaussian + quadratic mean
    RooRealVar a2("a2","a2",1,0.5,2);
    RooPolyVar fy2("fy2","fy2",y,RooArgSet(a0,a1,a2));
    RooGaussian quad_model("quad_model","Gaussian with shifting mean",x,fy2,sigma);
    TH2F* quad_response_hist = dynamic_cast<TH2F*>(quad_model.createHistogram("x,y"));
    
    
    // Toy Response gaussian + linear width
    RooRealVar b0("b0","b0",0,-5,5);
    RooRealVar b1("b1","b1",0.1,0.5,2);
    RooPolyVar fw("fw","fw",y,RooArgSet(b0,b1));
    
    RooGaussian width_model("width_model","Gaussian with shifting width",x,fy,fw);
    TH2F* width_response_hist = dynamic_cast<TH2F*>(width_model.createHistogram("x,y"));
    // Toy Truth Single Gaussian
    RooRealVar mean("mean","mean of toy signal", 0.25*(x_max-x_min));
    RooGaussian gauss("gauss","Gaussian of toy signal",x,mean,sigma);
    // Toy Truth Double Gaussian 
    RooRealVar mean2("mean2","second mean",0.75*(x_max-x_min));
    RooGaussian gauss2("gauss2","Gaussian of toy signal",x,mean2,sigma);
    RooRealVar frac("frac","Fraction",0.34,0,1);
    RooAddPdf toy2("toy2","toy Signal",RooArgList(gauss,gauss2),frac);
    // Monte Carlo Truth
    TH1D* truth = dynamic_cast<TH1D*>(make_normal_hist(base_hist,tree,name,"","_truth"));
    TH1D* reco = fold_truth(truth,response_matrix,171000);
    RooUnfoldBayes unfold(&response,reco,n_itr);
    TH1D* unfolded = dynamic_cast<TH1D*>(unfold.Hreco(RooUnfold::kCovariance));
    print_closure_plot(response_hist,truth,reco,unfolded);
  }
  out_file.Write();
  out_file.Close();
    
  return 0;
}
