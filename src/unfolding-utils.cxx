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
#include "TROOT.h"
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
    TMatrixD M(m,n);
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
  TH1D* rec_hist = dynamic_cast<TH1D*>(truth_hist->Clone(name.c_str()));
  rec_hist->Reset();
  
  for(int i =0; i <= rec_hist->GetNbinsX(); i++){
    rec_hist->SetBinContent(i,rec_vec[i]);
    rec_hist->SetBinError(i,TMath::Sqrt(rec_vec[i]));
  }
  return rec_hist;
}
void print_closure_plot(TH2* response_hist, TH1D* truth, TH1D* reco, TH1* unfolded, const std::string& suffix){
  aesthetic truth_aes=hist_aes("Truth",TColor::GetColor(8,81,156),0,kSolid);
  aesthetic reco_aes=hist_aes("Reconstructed",TColor::GetColor(165,15,21),0,kSolid);
  aesthetic unfolded_aes=data_aes("Unfolded",TColor::GetColor(0,0,0),kFullCircle,kSolid);
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
  unfolded->Draw("e1 x0 same");
  double max(std::max(std::max(truth->GetMaximum(),reco->GetMaximum()),unfolded->GetMaximum()));
  truth->SetMaximum(1.1*max);
  leg->Draw("lp");
  canv.cd();
  canv.SaveAs((std::string("uc_")+truth->GetName()+suffix+".pdf").c_str());
}

TH2D* transpose_hist(TH2* hist){
  TH2D* transposed = new TH2D(hist->GetName(),hist->GetTitle(),
			      hist->GetYaxis()->GetNbins(),
			      hist->GetYaxis()->GetXmin(),
			      hist->GetYaxis()->GetXmax(),
			      hist->GetXaxis()->GetNbins(),
			      hist->GetXaxis()->GetXmin(),
			      hist->GetXaxis()->GetXmax());
  double bc;
  double berr;
  for(int i=0; i < hist->GetXaxis()->GetNbins(); i++){
    for(int j=0; j < hist->GetYaxis()->GetNbins(); j++){
      bc=hist->GetBinContent(i,j);
      berr=hist->GetBinError(i,j);
      transposed->SetBinContent(j,i,bc);
      transposed->SetBinError(j,i,berr);
    }
  }
  return transposed;
}

void unfold_toy(TH2* (*make_response)(TH1*,TTree*,const int),
		TH1D* make_truth(TH1D*,TTree*,const int),
		TH1D* base_hist, TTree* tree, 
		const int n_itr, const int n_evts,
		const std::string& suffix){
  const std::string name(base_hist->GetName());
  TH2* response_hist=make_response(base_hist,tree,2e5);
  response_hist->SetName((name+suffix+"_rsp").c_str());

  TH1D truth_base(base_hist->GetName(),base_hist->GetTitle(),
  		  response_hist->GetNbinsX(),
  		  base_hist->GetXaxis()->GetXmin(),
  		  base_hist->GetXaxis()->GetXmax());
  
  TH1D* truth_hist = make_truth(&truth_base,tree,n_evts);
  TH1D* reco = fold_truth(truth_hist,norm_hist_to_mat(response_hist));
  TH1* unfolded = unfold(response_hist, reco, n_itr, name);
  double err_chg(0);
  for(int i =0 ; i < truth_hist->GetNbinsX(); i++){
    
    err_chg = truth_hist->GetBinError(i) > 0 ? (unfolded->GetBinError(i)-truth_hist->GetBinError(i))/truth_hist->GetBinError(i) : 0 ;
    MSG_DEBUG(truth_hist->GetBinCenter(i)<<" "<<err_chg);
  }
  print_closure_plot(response_hist,truth_hist,reco,unfolded,suffix);
}

TH1* unfold(TH2* response_hist, TH1* reco, int n_itr, const std::string& name){
  RooUnfoldResponse response(NULL, NULL,response_hist,
			     (name+"_unfolded").c_str(),reco->GetTitle());
  RooUnfoldBayes unfold(&response,reco,n_itr);
  unfold.SetVerbose(0);
  TH1D* unfolded = dynamic_cast<TH1D*>(unfold.Hreco(RooUnfold::kCovariance));
  return unfolded;
}
int get_iterations(TH1* base_hist,TH2* response_hist,const int n_evts){
  TH1D truth_base(base_hist->GetName(),base_hist->GetTitle(),
  		  response_hist->GetNbinsX(),
  		  base_hist->GetXaxis()->GetXmin(),
  		  base_hist->GetXaxis()->GetXmax());
  TH1D* truth = gauss_truth(&truth_base,NULL,n_evts);
  TH1D* reco = fold_truth(truth,norm_hist_to_mat(response_hist));
  double chi2_old(1);
  double chi2(0);
  int num_iter(1);
  chi2_old = reco->Chi2Test(truth,"CHI2"); 
  TH1* unfold_old=reco;
  MSG_DEBUG(chi2_old);
  int nbins = reco->GetNbinsX();
  while(true){
    TH1* unfolded = unfold(response_hist,reco,num_iter,base_hist->GetName());
    chi2 = unfolded->Chi2Test(unfold_old,"CHI2");
    MSG_DEBUG("Iteration: "<<num_iter<<" "<<chi2/nbins<<" delta chi2: "<<chi2_old-chi2);
    if(chi2/nbins < 1){
      break;
    }
    unfold_old = unfolded;
    chi2_old=chi2;
    num_iter++;
  };
  // double chi2_arr[50];
  // for(int i =1; i < 50; i++){
  //   TH1D* unfolded = unfold(response_hist,reco,num_iter,base_hist->GetName());
  //   chi2 = unfolded->Chi2Test(unfold_old,"CHI2");
  //   chi2_arr[i]=chi2;
  //   // MSG_DEBUG("Iteration: "<<num_iter<<" "<<chi2<<" delta chi2: "<<chi2_old-chi2);
  //   unfold_old = unfolded;
  //   chi2_old=chi2;
  //   num_iter++;
  // }
  // for(int i =0; i < 49; i++){
  //   MSG_DEBUG(i<<"\t"<<chi2_arr[i]-chi2_arr[i+1]);
  // }
  return num_iter;
}
TH1* build_unfold_err_hist(TH1* unfolded, TH2* response_hist, int num_iter,const std::string& name){
  TH1D* toy_hist = dynamic_cast<TH1D*>(unfolded->Clone((std::string(unfolded->GetName())+"_toy").c_str()));
  toy_hist->Reset();
  toy_hist->FillRandom(unfolded,unfolded->Integral());
  TH1* reco = fold_truth(toy_hist,norm_hist_to_mat(response_hist));
  TH1* unfold_toy = unfold(response_hist,reco,num_iter,name);
  unfold_toy->Add(toy_hist,-1);
  double bc;
  for(int i=0; i < unfold_toy->GetNbinsX(); i++){
    bc=unfold_toy->GetBinContent(i);
    unfold_toy->SetBinContent(i,0);
    unfold_toy->SetBinError(i,bc);
  }
  return unfold_toy;
}
TH1* unfold_syst_err(TH1* reco_hist,TTree* tree, 
		     const std::string& name,
		     const char* num_iter_str,
		     const std::string& suffix){
  int num_iter=-1;
  num_iter = atoi(num_iter_str);
  if(num_iter < 0){
    MSG_ERR("Unable to parse number of iterations, exiting");
    exit(num_iter);
  }
  TH1* unfolded;
  TDirectory *staging = gROOT->mkdir("staging");
  {
    // Set staging to be the current directory, return to the previous
    // current directory at the end of the scope
    TDirectory::TContext ctxt(staging); 
    TH1* base_hist = dynamic_cast<TH1*>(reco_hist->Clone(name.c_str()));
    base_hist->Reset();
    TH2* response_hist = mc_response(base_hist,tree,tree->GetEntries());
    unfolded =  unfold(response_hist,reco_hist,num_iter,name);
    unfolded->SetDirectory(nullptr);
    response_hist->SetDirectory(nullptr);
    base_hist->SetDirectory(nullptr);
  }
  return unfolded;
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
TH1D* gauss_truth(TH1D* base_hist, TTree* tree, const int n_evts){
  if(tree){};
  const std::string name(base_hist->GetName());

  int N_bins(base_hist->GetXaxis()->GetNbins());
  double x_min(base_hist->GetXaxis()->GetXmin());
  double x_max(base_hist->GetXaxis()->GetXmax());
  
  RooRealVar x("x","x",x_min,x_max);
  x.setBins(N_bins);
  
  RooRealVar mean("mean","mean of gaussian",0.5*(x_max-x_min)+x_min);
  RooRealVar sigma("sigma","width of gaussian",0.1*(x_max-x_min));
  RooGaussian model("model","Gaussian with shifting mean",x,mean,sigma);
  RooDataHist* toy_data = model.generateBinned(RooArgSet(x),n_evts); 
  TH1F* hist = dynamic_cast<TH1F*>(toy_data->createHistogram("x"));
  TH1D* result = dynamic_cast<TH1D*>(base_hist->Clone(name.c_str()));
  for(int i=0; i <= hist->GetNbinsX(); i++){
    result->SetBinContent(i,hist->GetBinContent(i));
    result->SetBinError(i,hist->GetBinError(i));
  }
  return result;
  
}
TH1D* dbl_gauss_truth(TH1D* base_hist, TTree* tree, const int n_evts){
  if(tree){};
  const std::string name(base_hist->GetName());

  int N_bins(base_hist->GetXaxis()->GetNbins());
  double x_min(base_hist->GetXaxis()->GetXmin());
  double x_max(base_hist->GetXaxis()->GetXmax());
  
  RooRealVar x("x","x",x_min,x_max);
  x.setBins(N_bins);
  
  RooRealVar m1("mean","mean of gaussian",0.2*(x_max-x_min)+x_min);
  RooRealVar m2("mean2","mean of gaussian",0.8*(x_max-x_min)+x_min);
  RooRealVar sigma("sigma","width of gaussian",0.1*(x_max-x_min));
  RooRealVar sigma2("sigma","width of gaussian",0.05*(x_max-x_min));
  RooGaussian gauss1("gauss1","Gaussian ",x,m1,sigma);
  RooGaussian gauss2("gauss2","Gaussian ",x,m2,sigma2);
  RooRealVar frac("frac","fraction",0.5,0,1);
  RooAddPdf model("model","Model",RooArgList(gauss1,gauss2),frac);
  RooDataHist* toy_data = model.generateBinned(RooArgSet(x),n_evts); 
  TH1F* hist = dynamic_cast<TH1F*>(toy_data->createHistogram("x"));
  TH1D* result = dynamic_cast<TH1D*>(base_hist->Clone(name.c_str()));
  for(int i=0; i <= hist->GetNbinsX(); i++){
    result->SetBinContent(i,hist->GetBinContent(i));
    result->SetBinError(i,hist->GetBinError(i));
  }
  return result;
  
}

TH2* mc_response(TH1* base_hist, TTree* tree, int n_evts){
  if(n_evts){};
  TH2D* response_hist = setup_response_hist(base_hist);
  return dynamic_cast<TH2D*>(make_response_hist(response_hist,tree,base_hist->GetName()));
}

TH2* linear_response_toy(TH1* base_hist,TTree* tree,int n_evts){
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

TH2* quad_response_toy(TH1* base_hist, TTree* tree, int n_evts){
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

TH2* width_response_toy(TH1* base_hist, TTree* tree, int n_evts){
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
