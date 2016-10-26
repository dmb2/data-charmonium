#include "unfolding-utils.hh"


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
#include "RooDataHist.h"

#include "histo-utils.hh"
#include "histo-style.hh"
#include "root-sugar.hh"

TMatrixD norm_hist_to_mat(TH2* h2){
    int n=h2->GetNbinsX()+2;
    int m=h2->GetNbinsY()+2;
    TMatrixD M(n,m);
    for(int i=0; i < n; i++){
      double norm=0;
      for(int j=0; j < m; j++){
	norm+=h2->GetBinContent(i,j);
      }
      for(int j=0; j < m; j++){
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
  MSG_DEBUG(response_matrix.GetNrows());
  MSG_DEBUG(response_matrix.GetNcols());
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
  double max(std::max(std::max(truth->GetMaximum(),reco->GetMaximum()),unfolded->GetMaximum()));
  truth->SetMaximum(1.1*max);
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
TH2F* linear_response_toy(TH1D* base_hist,int n_evts){
  int N_bins(base_hist->GetXaxis()->GetNbins());
  double x_min(base_hist->GetXaxis()->GetXmin());
  double x_max(base_hist->GetXaxis()->GetXmax());
  
  RooRealVar x("x","x",x_min,x_max);
  x.setBins(N_bins);
  RooRealVar y("y","y",x_min,x_max);
  y.setBins(N_bins);
  
  RooRealVar a0("a0","a0",x_min,-1e6,1e6);
  RooRealVar a1("a1","a1",1,0.5,2);
  //a0 + a1*y
  RooPolyVar fy("fy","fy",y,RooArgSet(a0,a1));
  RooRealVar sigma("sigma","width of gaussian",0.1*(x_max-x_min));
  RooGaussian model("model","Gaussian with shifting mean",x,fy,sigma);
  RooDataHist* toy_data = model.generateBinned(RooArgSet(x,y),n_evts); 
  return dynamic_cast<TH2F*>(toy_data->createHistogram("x,y"));
}
/*
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
*/
