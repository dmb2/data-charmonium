#include <map>
#include <string>
#include <iostream>
#include <cmath>

#include "TTree.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TStyle.h"
#include "TLatex.h"
#include "TColor.h"
#include "TGaxis.h"

#include "histo-utils.hh"
#include "root-sugar.hh"
#include "math.hh"
#include "AtlasStyle.hh"

void setup_global_style(){
  AtlasStyle style;
  style.SetAtlasStyle();
  gStyle->SetFrameLineWidth(0.0);
  gStyle->SetTitleYOffset(1.6);
  gStyle->SetLegendTextSize(0.03);
  TGaxis::SetMaxDigits(4);
}

void ensure_sumw2(TH1* hist){
  if(hist->GetSumw2N()==0){
    hist->Sumw2();
  }
}

void setup_hist(TH1* hist){
  hist->Sumw2();
  hist->SetMarkerStyle(1);
  hist->SetLineWidth(1.);
}
TH2D* setup_res_dif_hist(TH1* hist){
  const TAxis* axis = hist->GetXaxis();
  std::string axis_label(axis->GetTitle());
  double ymin=-50,ymax=125;
  if( std::string(hist->GetName())=="jet_eta" ||
      std::string(hist->GetName())=="jet_z" ){
    ymin=-0.4; ymax=0.4;
  }
  TH2D* hist2D = new TH2D((std::string(hist->GetName())+ "res_dif").c_str(), 
			  hist->GetTitle(), 
			  axis->GetNbins(), axis->GetXmin(), axis->GetXmax(),
			  50,ymin,ymax);
  hist2D->GetXaxis()->SetTitle(("Truth "+axis_label).c_str());
  hist2D->GetYaxis()->SetTitle(("Reco - Truth "+axis_label).c_str());
  return hist2D;
}
TH2D* setup_res_vtxz_hist(TH1* hist){
  const TAxis* axis = hist->GetXaxis();
  std::string axis_label(axis->GetTitle());
  double ymin=-50,ymax=125;
  if(std::string(hist->GetName())=="jet_eta" ||
     std::string(hist->GetName())=="jet_z" ){
    ymin=-0.4; ymax=0.4;
  }
  TH2D* hist2D = new TH2D((std::string(hist->GetName())+ "res_vtxz").c_str(), 
			  hist->GetTitle(), 50, -200, 200,50,ymin,ymax);
  hist2D->GetXaxis()->SetTitle("J/#psi vertex z [mm]");
  hist2D->GetYaxis()->SetTitle(("Reco - Truth "+axis_label).c_str());
  return hist2D;
}

TH2D* setup_rel_res_hist(TH1* hist){
  const TAxis* axis = hist->GetXaxis();
  std::string axis_label(axis->GetTitle());
  TH2D* hist2D = new TH2D((std::string(hist->GetName())+ "rel_rsp").c_str(), hist->GetTitle(),
			  axis->GetNbins(), axis->GetXmin(), axis->GetXmax(),
			  50,-0.8,1.0);
  hist2D->GetXaxis()->SetTitle(("Truth "+axis_label).c_str());
  hist2D->GetYaxis()->SetTitle("(Reco - Truth) / Truth");
  return hist2D;
}

TH2D* setup_response_hist(TH1* hist){
  TColor* color = new TColor(1756,0.0,0.0,0.0,"tran_black",0.75);
  const TAxis* axis = hist->GetXaxis();
  TH2D* hist2D = new TH2D((std::string(hist->GetName())+ "_rsp").c_str(), hist->GetTitle(),
			  axis->GetNbins(), axis->GetXmin(), axis->GetXmax(),
			  axis->GetNbins(), axis->GetXmin(), axis->GetXmax());
  hist2D->SetMarkerStyle(6);
  hist2D->SetMarkerColor(color->GetNumber());

  hist2D->GetYaxis()->SetTitle("Truth");
  hist2D->GetXaxis()->SetTitle("Reconstructed");
  delete color;
  return hist2D;
}
void draw_histo(TTree* tree,const char* branch_name, const char* hist_name, 
		const char* cut_expr){
  char branch_expr[500];
  snprintf(branch_expr,LEN(branch_expr),"%s>>%s", branch_name,hist_name);
  // MSG_DEBUG(cut_expr);
  // MSG_DEBUG(branch_expr);
  tree->Draw(branch_expr,cut_expr,"goff");
}
void remove_axis(TAxis* axis){
  axis->SetLabelSize(0);
  axis->SetTitle("");
  axis->SetLabelOffset(999);
}
TLegend* init_legend(double x1,double y1, double x2,double y2){
  TLegend* leg = new TLegend(x1,y1,x2,y2);
  leg->SetFillColor(0);
  leg->SetFillStyle(0);
  leg->SetBorderSize(0);
  return leg;
}

void set_pad_margins(TVirtualPad* pad,int pad_pos, int N_hists,int n_col,int n_row, bool y_axis){
  pad->SetRightMargin(0);
  pad->SetTopMargin(0.05);
  if((pad_pos-1)%n_col!=0 && !y_axis){
    pad->SetLeftMargin(0);
  }
  if (pad_pos-1 < (n_row - 2)*n_col + N_hists%n_col){
    pad->SetBottomMargin(0);
  }
}
TH1* make_response_hist(TH1* base_hist, TTree* tree, 
			const std::vector<std::string>& cut_branches,size_t cut_index, 
			const std::string& plot){
  TH1* response = (TH1*)base_hist->Clone((plot + "_RSP_"+
					  str_join("_",cut_branches,
						   0,cut_index+1)).c_str());
  draw_histo(tree,("truth_"+plot+":"+plot).c_str(),response->GetName(),
  	     str_join("*",cut_branches,0,cut_index+1).c_str());
  return response;
}
TH1* make_normal_hist(TH1* base_hist, TTree* tree, 
		      const std::vector<std::string>& cut_branches, size_t cut_index, 
		      const std::string& plot){
  //TODO use Directory staging
  static int counter=0;
  counter++;
  char count_str[10];
  snprintf(count_str,LEN(count_str),"%d",counter);
  std::string uniq_suffix=str_join("_",cut_branches,0,cut_index+1);
  if(uniq_suffix.find("&&")!=std::string::npos){
    uniq_suffix="signal_region";
  }
  const std::string weight_expr=cut_branches.size()==0 ? "pileup_weight*SF*weight" :
    "pileup_weight*SF*weight*"+str_join("*",cut_branches,0,cut_index+1);
  // MSG_DEBUG(weight_expr);
  return make_normal_hist(base_hist,tree,plot,
			  weight_expr.c_str(),"_NRM_"+std::string(count_str)+uniq_suffix);
}
TH1* make_ratio_hist(TH1* base_hist, TTree* tree,
		     const std::vector<std::string>& cut_branches,size_t cut_index, 
		     const std::string& plot){
  TH1* ratio =(TH1*)base_hist->Clone((plot +"_R_"+
				      str_join("_",cut_branches,
					       0,cut_index+1)).c_str());
  std::string hNDenom = plot+str_join("_",cut_branches,0,cut_index)+"_tmp";
  TH1* hDenom = (TH1*)ratio->Clone(hNDenom.c_str());
  draw_histo(tree, plot.c_str(), hNDenom.c_str(),
	     str_join("*", cut_branches, 0,cut_index).c_str());
  draw_histo(tree, plot.c_str(), ratio->GetName(),
	     str_join("*", cut_branches, 0,cut_index+1).c_str());
  ratio->Divide(hDenom);
  ratio->SetMaximum(1.2);
  ratio->SetMinimum(0.);
  return ratio;
}
TH1* make_res_vtxz_hist(TH1* base_hist,TTree* tree,const std::string& plot,
			const char* weight_expr, const std::string& name_suffix){
  std::string draw_expr = plot+"-"+"truth_"+plot;
  // MSG_DEBUG(draw_expr+":truth_"+plot);
  TH1* hist = (TH1*)base_hist->Clone((plot + name_suffix+"_res_vtxz").c_str());
  draw_histo(tree,(draw_expr+":jpsi_vtx_z").c_str(), hist->GetName(), weight_expr);
  return hist;
}
TH1* make_res_dif_hist(TH1* base_hist,TTree* tree,const std::string& plot,
			const char* weight_expr, const std::string& name_suffix){
  std::string draw_expr = plot+"-"+"truth_"+plot;
  // MSG_DEBUG(draw_expr+":truth_"+plot);
  TH1* hist = (TH1*)base_hist->Clone((plot + name_suffix+"_res_dif").c_str());
  draw_histo(tree,(draw_expr+":truth_"+plot).c_str(), hist->GetName(), weight_expr);
  return hist;
}

TH1* make_rel_res_hist(TH1* base_hist,TTree* tree,const std::string& plot,
			const char* weight_expr, const std::string& name_suffix){
  // Suppress compiler warnings, yes that is what I want.
  if(name_suffix != ""){};
  std::string draw_expr = "("+plot+"- truth_"+plot+")/truth_"+plot;
  TH1* hist = (TH1*)base_hist->Clone((plot + "_rel_res").c_str());
  draw_histo(tree,(draw_expr+":truth_"+plot).c_str(), hist->GetName(), weight_expr);
  return hist;
}

TH1* make_response_hist(TH1* base_hist,TTree* tree,const std::string& plot,
			const char* weight_expr, const std::string& name_suffix){
  TH1* hist = (TH1*)base_hist->Clone((plot + name_suffix+"_rsp_NOM").c_str());
  draw_histo(tree,("truth_"+plot + ":"+plot).c_str(), hist->GetName(), weight_expr);
  return hist;
}
TH1* make_normal_hist(TH1* base_hist,TTree* tree,const std::string& plot,
		      const char* weight_expr,
		      const std::string& name_suffix){
  TH1* hist = (TH1*)base_hist->Clone((plot + name_suffix).c_str());
  draw_histo(tree,plot.c_str(), hist->GetName(), weight_expr);
  return hist;
}
void add_atlas_badge(TVirtualPad& canv,const double x, const double y, 
		     const double lumi_fb, const status_t status){
  canv.cd();
  std::string status_label="";

  if(status==PRELIMINARY){
    status_label="Preliminary";
  } 
  else if(status==INTERNAL){
    status_label="Internal";
  } 
  else if(status==APPROVED){
    status_label="Approved";
  } 
  else if(status==WIP){
    status_label="Work In Progress";
  } else {
    status_label="";
  }
  
  // canv.cd();
  Double_t tsize(0.04);
  TLatex l; 
  l.SetTextSize(tsize); 
  l.SetNDC();
  l.SetTextFont(72);

  //what sorcery is this?
  double delx = 0.1*696*gPad->GetWh()/(472*gPad->GetWw());
  double dely = (tsize+0.005) *696*gPad->GetWw()/(472*gPad->GetWh());
	
  l.DrawLatex(x,y,"ATLAS");
  l.SetTextFont(42);
  l.DrawLatex(x+delx,y,status_label.c_str());
  char data_cond[70];
  if(lumi_fb > 0){
    const char* fmt_str = "#sqrt{s} = 8 TeV; #int L dt = %.3g fb^{-1}";
    snprintf(data_cond,sizeof(data_cond)/sizeof(*data_cond),fmt_str,lumi_fb);
  }
  else{
    const char* fmt_str = "#sqrt{s} = 8 TeV";
    snprintf(data_cond,sizeof(data_cond)/sizeof(*data_cond),fmt_str);
  }
  l.DrawLatex(x,y-dely,data_cond);
}

static void style_err_hist(TH1D* hist,int color){
  hist->SetFillColor(color);
  hist->SetMarkerStyle(kDot);
  hist->SetMarkerSize(0);
}
// TODO Either remove, or refactor
void print_profile_hist(TH1* base_hist,TTree* tree,const std::string& plot,
			const std::string& suffix,
			TH1* (*make_hist)(TH1*,TTree*,
					  const std::string&,const char*,
					  const std::string&)){
  // Project to a list of hists along x, gather mean, mean error,
  // stddev, stddev err and make a TH1D that has a x and y axis
  // corresponding to the profile.
  TH2D* hist2D = dynamic_cast<TH2D*>(make_hist(base_hist, tree, plot, "truth_jet_pt > 45", "_prof_"));
  TAxis* x_axis = hist2D->GetXaxis();
  TH1D mean_hist((base_hist->GetName()+std::string("_mean")).c_str(), base_hist->GetTitle(),
	      x_axis->GetNbins(), x_axis->GetXmin(), x_axis->GetXmax());
  TH1D* std_dev_hist=dynamic_cast<TH1D*>(mean_hist.Clone((base_hist->GetName()+ std::string("std_dev")).c_str()));
  TH1D* sigma_up_hist=dynamic_cast<TH1D*>(mean_hist.Clone((base_hist->GetName()+ std::string("sigma_up")).c_str()));
  TH1D* sigma_down_hist=dynamic_cast<TH1D*>(mean_hist.Clone((base_hist->GetName()+ std::string("sigma_down")).c_str()));
  TH1D* hist=NULL;
  for(size_t i= 1; i < (size_t)hist2D->GetNbinsX()+1; i++){
    hist = hist2D->ProjectionY("_px",i,i+1,"e");
    mean_hist.SetBinContent(i,hist->GetMean());
    mean_hist.SetBinError(i,hist->GetMeanError());
    std_dev_hist->SetBinContent(i,0);
    std_dev_hist->SetBinError(i,hist->GetStdDev());
    sigma_up_hist->SetBinContent(i,hist->GetMean()+hist->GetStdDevError());
    sigma_up_hist->SetBinError(i,hist->GetStdDev());
    sigma_down_hist->SetBinContent(i,hist->GetMean()-hist->GetStdDevError());
    sigma_down_hist->SetBinError(i,hist->GetStdDev());
    // MSG_DEBUG(hist->GetMean()<<" \\pm "<<hist->GetMeanError()<<" "<<hist->GetStdDev()<<" \\pm"<<hist->GetStdDevError());
  }
  TCanvas canv(("canv_"+plot).c_str(),"Plot",600,600);
  TLatex decorator;
  decorator.SetTextSize(0.04);
  std_dev_hist->Add(&mean_hist);
  int tred = TColor::GetColorTransparent(kRed,0.3);
  style_err_hist(std_dev_hist,tred);
  style_err_hist(sigma_up_hist,tred);
  style_err_hist(sigma_down_hist,tred);
  std_dev_hist->GetYaxis()->SetTitle(hist2D->GetYaxis()->GetTitle());
  std_dev_hist->GetXaxis()->SetTitle(hist2D->GetXaxis()->GetTitle());
  std_dev_hist->Draw("e4");
  std_dev_hist->GetXaxis()->GetTitle();
  sigma_down_hist->Draw("e4 same");
  sigma_up_hist->Draw("e4 same");
  mean_hist.Draw("e1 same");
  decorator.DrawLatexNDC(0.,0.05,base_hist->GetTitle());
  canv.SaveAs((plot+suffix).c_str());
}
void print_hist(TTree* tree, const std::string& plot, 
		TH1* base_hist, 
		const std::vector<std::string>& cut_branches, 
		const std::string suffix, 
		TH1* (*make_hist)(TH1*,TTree*,const std::string&, const char*,
				  const std::string&)){
  TCanvas canv(("canv_"+plot).c_str(),"Plot",600,600);
  TLatex decorator;
  decorator.SetTextSize(0.04);
  MSG_DEBUG("Warning: using truth_jet_pt cut!");
  const std::string weight_expr="pileup_weight*SF*weight*(" +
    str_join("*",cut_branches, 0,cut_branches.size())+" && truth_jet_pt > 45)";
  // MSG_DEBUG(weight_expr);
  TH1* hist = make_hist(base_hist,tree,plot,weight_expr.c_str(),"_nom");
  hist->Draw("H COLZ");
  decorator.DrawLatexNDC(0.,0.05,hist->GetTitle());
  canv.SaveAs((plot+suffix).c_str());
}
TH1* build_syst_err_hist(TH1* base_hist, const std::string& samp_name,
			 const char* cut_expr, 
			 TH1* (make_var_hist)(TH1*,TTree*, const std::string&,
					      const char*,const std::string&)){
  std::map<std::string,std::string> syst_map;
  syst_map["MuonEfficiency"]="";
  syst_map["MuonSmearedUp"]="MuonSmearedLow";
  syst_map["TrackZRadialScaledUpJPsiJets"]="TrackZRadialScaledDownJPsiJets";
  syst_map["TrackZScaledUpJPsiJets"]="TrackZScaledDownJPsiJets";
  syst_map["MuonSmearedIDUp"]="";
  syst_map["MuonSmeared"]="";
  syst_map["MuonSmearedMSUp"]="";
  syst_map["TrackZFilteredJPsiJets"]="";
  syst_map["TrackZSmearedJPsiJets"]="";
  char fname[256];
  snprintf(fname,LEN(fname),"%s.mini.root",samp_name.c_str());
  TTree* tree = retrieve<TTree>(fname,"mini");
  TH1* tot_err = dynamic_cast<TH1*>(base_hist->Clone((base_hist->GetName()+samp_name+"_tot_err").c_str()));
  std::string dsid=split_string(samp_name,'.').at(0);
  std::string syst_cut_expr(cut_expr);
  // tot_err->Reset();
  for(int i =0; i < tot_err->GetNbinsX(); i++){
    tot_err->SetBinContent(i,0);
    tot_err->SetBinError(i,0);
  }

  for(std::map<std::string,std::string>::const_iterator it = syst_map.begin();
      it!=syst_map.end(); ++it){
    const std::string& var_up = it->first;
    const std::string& var_down = it->second;
    // MSG_DEBUG("Variation: "<<var_up<<" and "<<var_down);
    double sf = var_down=="" ? 1.0 : 0.5;
    bool mu_eff=(var_up=="MuonEfficiency");
    TTree* up_tree=nullptr;
    TTree* down_tree = nullptr;
    char up_cut_expr[256];
    char down_cut_expr[256];

    //This is kind of hairy, but here's the rough idea: Every
    // systematic variation has its own file except for the muon
    // efficiency. This systematic is applied as a weight to the
    // distribution. To assess the uncertainty the weight is varied up
    // and down according to SFTotalErr. Therefore, for our code we
    // set up the trees to point to the same (nominal) tree, and
    // modifiy the cut_expression to be weighted according to the
    // variation's error, up or down. If we aren't assessiong the muon
    // efficiency, we retrieve the trees and leave the cut expression
    // alone.
    if(mu_eff){
      up_tree = tree;
      down_tree = tree;
      snprintf(up_cut_expr,LEN(up_cut_expr),"%s*(1+SFTotalErr)",cut_expr);
      snprintf(down_cut_expr,LEN(down_cut_expr),"%s*(1-SFTotalErr)",cut_expr);
    } else {
      snprintf(up_cut_expr,LEN(up_cut_expr),"%s",cut_expr);
      snprintf(down_cut_expr,LEN(down_cut_expr),"%s",cut_expr);
      snprintf(fname,LEN(fname),"%s-systematics/%s.%s.mini.root",dsid.c_str(),dsid.c_str(),var_up.c_str());
      TFile* file = TFile::Open(fname);
      file->GetObject("mini",up_tree);
      if(var_down!=""){
	snprintf(fname,LEN(fname),"%s-systematics/%s.%s.mini.root",dsid.c_str(),dsid.c_str(),var_down.c_str());
	file = TFile::Open(fname);
	file->GetObject("mini",down_tree);
      }
      else{
	down_tree=tree;
      }
    }
    if(up_tree==nullptr || down_tree == nullptr){
      MSG_ERR("Systematic tree was not retrieved, exiting!");
      exit(-1);
    }
    // If the original cut_expr is less than 3, it is the number of
    // iterations for unfolding. Therefore we override it again so
    // that the function pointer make_var_hist can properly parse the
    // string into the number of iterations
    if(std::string(cut_expr).size() < 3){
      snprintf(up_cut_expr,LEN(up_cut_expr),"%s",cut_expr);
      snprintf(down_cut_expr,LEN(down_cut_expr),"%s",cut_expr);
    }
    // MSG_DEBUG("Up cut expression: "<<up_cut_expr);
    // MSG_DEBUG("Down cut expression: "<<down_cut_expr);
    // MSG_DEBUG("Up Tree: "<<up_tree<<" and down tree: "<<down_tree);
    TH1* syst_up_hist = make_var_hist(base_hist,up_tree,base_hist->GetName(),
				      up_cut_expr,
				      samp_name+"_syst_up");
    syst_up_hist->SetName((base_hist->GetName()+var_up).c_str());
    TH1* syst_down_hist = make_var_hist(base_hist,down_tree,base_hist->GetName(),
					down_cut_expr,
					samp_name+"_syst_down");
    syst_down_hist->SetName((base_hist->GetName()+var_down).c_str());
    // dump_hist(syst_down_hist);
    // dump_hist(syst_up_hist);
    syst_up_hist->Add(syst_down_hist,-1.0);
    syst_up_hist->Scale(sf);
    double bc;
    for(int i=0; i <syst_up_hist->GetNbinsX(); i++){
      bc=syst_up_hist->GetBinContent(i);
      syst_up_hist->SetBinContent(i,0);
      syst_up_hist->SetBinError(i,fabs(bc));
    }
    add_err(tot_err,syst_up_hist);
  }
  return tot_err;
}

void scale_errors(TH1* hist){
  double err(0);
  double content(0);
  for(int i=0; i <= hist->GetNbinsX(); ++i){
    err=hist->GetBinError(i);
    content=hist->GetBinContent(i);
    // MSG_DEBUG("err: "<<err<< " content: "<<content);
    // if(content > 0 && err/content > 1.2 && content < 1){
    //   MSG_ERR("Error greater than content, truncating to 0");
    //   MSG_DEBUG("err: "<<err<< " content: "<<content);
    //   content=0;
    // }
    hist->SetBinError(i,0);
    hist->SetBinContent(i,content > 0 ? err/content : 0);
    // MSG_DEBUG("new content: "<<hist->GetBinContent(i));
  }
}
void add_bc(TH1* hista, TH1* histb){
  if(hista->GetNbinsX()!=histb->GetNbinsX()){
    MSG_ERR("Bin size mismatch: "<<hista->GetNbinsX()<<" vs "<<histb->GetNbinsX());
    return;
  }
  double val;
  for(int i=0; i < hista->GetNbinsX(); i++){
    // MSG_DEBUG("a: "<<str_rep(a)<< " b: "<<str_rep(b)<<" a+b: "<<str_rep(add(a,b)));
    val = hista->GetBinContent(i)+histb->GetBinContent(i);
    hista->SetBinContent(i,val);
  }
}
void add_err(TH1* hista, TH1* histb){
  if(hista->GetNbinsX()!=histb->GetNbinsX()){
    MSG_ERR("Bin size mismatch: "<<hista->GetNbinsX()<<" vs "<<histb->GetNbinsX());
    return;
  }
  num_err a; num_err b;
  for(int i=0; i <= hista->GetNbinsX(); i++){
    a.val=0; a.err=hista->GetBinError(i);
    b.val=0; b.err=histb->GetBinError(i);
    // MSG_DEBUG("a: "<<str_rep(a)<< " b: "<<str_rep(b)<<" a+b: "<<str_rep(add(a,b)));
    hista->SetBinError(i,add(a,b).err);
  }
}
bool has_non_zero_error(TH1* hist){
  // ignore overflow/underflow intentionally
  for(int i=1; i < hist->GetNbinsX(); i++){
    if(hist->GetBinError(i) != 0){
      return true;
    }
  }
  return false;
}

void print_cut_hist(TTree* tree, const std::vector<std::string>& cut_branches,
		const std::string& plot, TH1* base_hist, 
		std::map<std::string,std::string>& CutNames, std::string file_suffix,
		    TH1* (*make_hist)(TH1* ,TTree* , const std::vector<std::string>&,size_t, const std::string&)){
  TCanvas canv(("canv_"+plot).c_str(),"Cut Plot",1800,800);
  // TPaveText label(0.08,0.6,0.18,0.71,"NB");
  TLatex decorator;
  // decorator.SetTextAlign(12);
  decorator.SetTextSize(0.1);
  canv.Divide(3,2);
  canv.SetRightMargin(0);
  canv.SetTopMargin(0);
  if(plot.find("pt")!=std::string::npos){
    canv.SetLogy(true);
  }
  size_t nCuts=cut_branches.size();
  for(size_t i = 0; i < nCuts; i++){
    set_pad_margins(canv.cd(i+1),i+1,nCuts);
    TH1* hist = make_hist(base_hist,tree, cut_branches, i,plot);
    hist->Draw("COLZ");
    if( i < 3){ //top row
      remove_axis(hist->GetXaxis());
    }
    if(i != 0 && i != 3 && file_suffix.find("normal")==std::string::npos){
      remove_axis(hist->GetYaxis());
    }
    decorator.DrawLatexNDC(0.5,0.75,CutNames[cut_branches[i]].c_str());
  }
  canv.cd(0);
  decorator.SetTextSize(0.04);
  decorator.DrawLatex(0.0,0.05,base_hist->GetTitle());
  canv.SaveAs((plot+file_suffix).c_str());
}
void print_corr_plot(TH1* base_hist, const std::string disc_name,
		     const int n_bins, const double min, const double max,
		     TTree* tree, const char* suffix,
		     const double lumi, const char* weight_expr){
  const std::string plot_name(base_hist->GetName());
  TH2D* hist = new TH2D((plot_name+disc_name+"_corr").c_str(),base_hist->GetTitle(),
			base_hist->GetNbinsX(), base_hist->GetXaxis()->GetXmin(),
			base_hist->GetXaxis()->GetXmax(),
			n_bins,min,max);
  hist->GetXaxis()->SetTitle(base_hist->GetXaxis()->GetTitle());
  hist->GetYaxis()->SetTitle(disc_name=="jpsi_tau" ? "Lifetime [ps]" : "Mass [GeV]");
  std::string draw_expr = disc_name+ std::string(":") + plot_name;
  draw_histo(tree,draw_expr.c_str(), hist->GetName(), weight_expr);
  TCanvas canv("canv","Correlation canvas",600,600);
  hist->Draw("COLZ");
  canv.SetRightMargin(0.16);
  TLatex decorator;
  decorator.SetTextSize(0.04);
  char corr_str[256];
  snprintf(corr_str,256,"Global Correlation: %.5g",hist->GetCorrelationFactor());
  MSG_DEBUG(corr_str);
  add_atlas_badge(canv,0.2,0.9,lumi);
  decorator.DrawLatexNDC(0.02,0.05,corr_str);
  // decorator.DrawLatexNDC(0.5,0.0,base_hist->GetTitle());
  canv.SaveAs((plot_name+suffix).c_str());
}

void dump_hist(const TH1* hist){
  MSG_DEBUG("Hist: "<<hist->GetName());
  MSG_DEBUG("Integral: "<<hist->Integral());
  MSG_DEBUG("Entries: "<<hist->GetEntries());
  for(int i =0; i < hist->GetNbinsX(); i++){
    MSG_DEBUG(i<<" "<<hist->GetBinContent(i)<<"+/-"<<hist->GetBinError(i));
  }
}
