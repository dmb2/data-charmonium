#include "unfolding-utils.hh"
#include <iomanip>


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
	norm+=h2->GetBinContent(j,i);
      }
      for(int j=0; j < m; j++){
	if(norm!=0){
	  M[j][i]=h2->GetBinContent(j,i)/norm;
	}
      }
    }
    return M;
}

TH1D* fold_truth(TH1D* truth_hist, const TMatrixD& response_matrix){
  const std::string name(truth_hist->GetName());
  TVectorD true_vec(truth_hist->GetNbinsX()+2,truth_hist->GetArray());
  TVectorD rec_vec = response_matrix*true_vec;
  // double t_tot;
  // double r_tot;
  // std::cout<<std::setw(10)<<"Truth "<<"Reco"<<std::endl;
  // for(int i=0; i < truth_hist->GetNbinsX()+2; i++){
  //   t_tot+=true_vec[i];
  //   r_tot+=rec_vec[i];
  //   std::cout<<std::setw(10)<<true_vec[i]<<" "<<rec_vec[i]<<std::endl;
  // }
  //     std::cout << "Truth norm: "<<t_tot<<" Reco norm: "<<r_tot<<std::endl;
  TH1D* rec_hist = dynamic_cast<TH1D*>(truth_hist->Clone(name.c_str()));
  rec_hist->Reset();
  for(int i =0; i <= rec_hist->GetNbinsX(); i++){
    rec_hist->SetBinContent(i,rec_vec[i]);
    rec_hist->SetBinError(i,TMath::Sqrt(rec_vec[i]));
    // std::cout<<rec_hist->GetBinContent(i)<<std::endl;
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
  canv.SaveAs((std::string("uc_")+truth->GetName()+suffix+".pdf").c_str());
}

void unfold_toy(TH2* (*make_response)(TH1D*,TTree*,const int),
		TH1D* make_truth(TH1D*,TTree*,const int),
		TH1D* base_hist, TTree* tree, 
		const int n_itr, const int n_evts,
		const std::string& suffix){
  const std::string name(base_hist->GetName());
  TH2* response_hist=make_response(base_hist,tree,n_evts);
  response_hist->SetName((name+"_response_hist").c_str());

  TH1D* truth_hist = make_truth(base_hist,tree,n_evts);
  TH1D* reco = fold_truth(truth_hist,norm_hist_to_mat(response_hist));
  TH1D* unfolded = unfold(response_hist,reco, n_itr, name);
  print_closure_plot(response_hist,truth_hist,reco,unfolded,suffix);
}

TH1D* unfold(TH2* response_hist, TH1D* reco, int n_itr, const std::string& name){
  RooUnfoldResponse response(NULL, NULL,response_hist,
			     (name+"_unfolded").c_str(),reco->GetTitle());
  RooUnfoldBayes unfold(&response,reco,n_itr);
  return dynamic_cast<TH1D*>(unfold.Hreco(RooUnfold::kCovariance));
}

TH1D* mc_truth(TH1D* base_hist, TTree* tree, const int n_evts){
  const std::string name(base_hist->GetName());
  TH1D* truth = dynamic_cast<TH1D*>(make_normal_hist(base_hist,tree,"truth_"+name));
  truth->SetName(name.c_str());
  TH1D* truth_hist = dynamic_cast<TH1D*>(truth->Clone(("truth_"+name).c_str()));
  truth_hist->Reset();
  truth_hist->FillRandom(truth,n_evts);
  return truth_hist;
}
TH2* mc_response(TH1D* base_hist, TTree* tree, int n_evts){
  if(n_evts){};
  TH2* response_hist = setup_response_hist(base_hist);
  return dynamic_cast<TH2D*>(make_response_hist(response_hist,tree,base_hist->GetName(),"","_response"));
}

TH2* linear_response_toy(TH1D* base_hist,TTree* tree,int n_evts){
  if(tree){};
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
  RooRealVar sigma("sigma","width of gaussian",0.05*(x_max-x_min));
  RooGaussian model("model","Gaussian with shifting mean",x,fy,sigma);
  RooDataHist* toy_data = model.generateBinned(RooArgSet(x,y),n_evts); 
  return dynamic_cast<TH2*>(toy_data->createHistogram("x,y"));
}

TH2* quad_response_toy(TH1D* base_hist, TTree* tree, int n_evts){
  if(tree){};
  int N_bins(base_hist->GetXaxis()->GetNbins());
  double x_min(base_hist->GetXaxis()->GetXmin());
  double x_max(base_hist->GetXaxis()->GetXmax());

  RooRealVar x("x","x",x_min,x_max);
  x.setBins(N_bins);
  RooRealVar y("y","y",x_min,x_max);
  y.setBins(N_bins);
  double delta_x = x_max - x_min;
  RooRealVar a0("a0","a0",x_min*x_min/delta_x+x_min,-1e6,1e6);
  RooRealVar a1("a1","a1",2*x_min/delta_x,-1e6,1e6);
  RooRealVar a2("a2","a2",1/delta_x,-1e6,1e6);
  RooRealVar sigma("sigma","width of gaussian",0.05*delta_x);
  // a0 + a1*y + a2*y^2
  RooPolyVar fy("fy2","fy2",y,RooArgSet(a0,a1,a2));
  RooGaussian model("quad_model","Gaussian with shifting mean",x,fy,sigma);
  
  RooDataHist* toy_data = model.generateBinned(RooArgSet(x,y),n_evts); 
  return dynamic_cast<TH2*>(toy_data->createHistogram("x,y"));
}

TH2* width_response_toy(TH1D* base_hist, TTree* tree, int n_evts){
  if(tree){};
  int N_bins(base_hist->GetXaxis()->GetNbins());
  double x_min(base_hist->GetXaxis()->GetXmin());
  double x_max(base_hist->GetXaxis()->GetXmax());

  RooRealVar x("x","x",x_min,x_max);
  x.setBins(N_bins);
  RooRealVar y("y","y",x_min,x_max);
  y.setBins(N_bins);
  RooRealVar a0("a0","a0",0,-5,5);
  RooRealVar a1("a1","a1",1,0.5,2);
  // a0 + a1*y
  RooPolyVar fy("fy","fy",y,RooArgSet(a0,a1));
  RooRealVar b0("b0","b0",0,-5,5);
  RooRealVar b1("b1","b1",0.1,0.5,2);
  // b0 + b1*y
  RooPolyVar fw("fw","fw",y,RooArgSet(b0,b1));
  RooGaussian model("model","Gaussian with shifting width",x,fy,fw);
  RooDataHist* toy_data = model.generateBinned(RooArgSet(x,y),n_evts); 
  return dynamic_cast<TH2*>(toy_data->createHistogram("x,y"));
}
