#include <string>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <map>

#include "TLegend.h"
#include "TPaletteAxis.h"
#include "TLatex.h"
#include "THStack.h"
#include "TColor.h"
#include "TList.h"
#include "TTree.h"
#include "TROOT.h"
#include "TCanvas.h"
#include "TH2D.h"

#include "stack-utils.hh"
#include "root-sugar.hh"
#include "histo-style.hh"
#include "histo-utils.hh"
#include "histo-meta-data.hh"

using std::string;
using std::map;

void scale_stack(TH1** hist_list, const size_t n, double sf){
  for(size_t i=0; i < n; i++){
    hist_list[i]->Scale(sf);
  }
}
THStack* make_stack(TH1* base_hist, std::map<std::string,TTree*>& samples,
		    const char* cut_branches[], int cut_index, 
		    const std::string& plot, TLegend& leg, double target_lumi ){
  std::vector<std::string> sample_names = map_keys(samples);
  std::map<std::string,aesthetic> hist_styles;
  init_hist_styles(hist_styles);
  THStack* stack = new THStack(("stack_"+plot).c_str(),base_hist->GetTitle());
  double total=0.;
  size_t num_hists=sample_names.size();
  TH1** hist_list = (TH1**)calloc(num_hists, sizeof(TH1*));
  string cut_expr;
  char cut_str[1024];
  for(size_t i=0; i < num_hists; i++){
    cut_expr.clear();
    const string name(sample_names[i]);
    if(name=="master"){
      continue;
    }

    TTree* const tree = samples[name];
    TH1* hist =(TH1*)base_hist->Clone((name+plot+"_"+cut_branches[cut_index]).c_str());
    hist_list[i]=hist;
    cut_expr=((cut_index == 0) ? "weight" : "weight*" 
	      + str_join("*",cut_branches,0,cut_index+1));
    cut_expr+="*%.4g";
    snprintf(cut_str,sizeof(cut_str)/sizeof(*cut_str),cut_expr.c_str(),target_lumi);
    draw_histo(tree,plot.c_str(),hist->GetName(), cut_str);
    total+=hist->Integral();
    stack->Add(hist);
    style_hist(hist,hist_styles[name]);
    add_to_legend(&leg,hist,hist_styles[name]);
  }
  //ROOT SUCKS
  // scale_stack(hist_list,num_hists-1,n_master > 0 ? n_master/total : 1);
  free(hist_list);
  return stack;
}
void print_2D_stack(std::map<std::string,TTree*> samples,const std::string& plot,
		    TH1* base_hist, const std::string& suffix, 
		    const double target_lumi){
  const size_t n_samp = samples.size();
  const size_t n_col = 3;
  //kind of dirty
  const size_t n_row = static_cast<size_t>(ceil(n_samp/(n_col+0.)));
  TCanvas canv(("2D_stk_canv_"+plot).c_str(),"2D Stack",600*n_col,600*n_row);
  TLatex decorator;
  std::map<std::string,aesthetic> hist_styles;
  init_hist_styles(hist_styles);
  std::vector<std::string> sample_names=map_keys(samples);
  decorator.SetTextSize(0.07);
  canv.Divide(n_col,n_row);
  // canv.cd(1);
  char weight_expr[256];
  snprintf(weight_expr,256,"weight*%.4g",target_lumi);
  for(size_t i=0; i < n_samp; i++){
    TVirtualPad* pad = canv.cd(i+1);
    // set_pad_margins(pad,i+1,n_samp,n_col,n_row,false);
    pad->SetRightMargin(0.16);
    std::string& name = sample_names[i];
    TH1* hist = make_normal_hist(base_hist,samples[name],plot,
				 weight_expr,name+"_2D_STK");
    canv.cd(i+1);
    hist->Draw("COLZ");
    /*
    if(i < (n_row - 2)*n_col + n_samp%n_col){
      remove_axis(hist->GetXaxis());
    }
    if(i%n_col!=0){
      remove_axis(hist->GetYaxis());
    }
    */
    decorator.DrawLatexNDC(0.25,0.25,hist_styles[name].leg_label);
  }
  canv.cd(0);
  decorator.SetTextSize(0.04);
  decorator.DrawLatex(0.01,0.02,base_hist->GetTitle());
  std::string outname=plot+suffix;
  replace(outname.begin(),outname.end(),':','_');
  canv.SaveAs(outname.c_str());
}
void norm_stack(THStack& stack){
  TIter next(stack.GetHists());
  TH1* h = NULL;
  while((h = dynamic_cast<TH1*>(next()))){
    h->Scale(1/h->Integral());
  }
}
static std::vector<double> build_norm_factors( const TH2D *HistZvsE){
  std::vector<double> result;
  result.reserve(HistZvsE->GetNbinsX());
  TH1D* hist=NULL;
  for(size_t i = 1; i < (size_t)HistZvsE->GetNbinsX()+1; i++){
    hist=HistZvsE->ProjectionY("_px",i,i+1,"e");
    result.push_back(hist->Integral());
  }
  return result;
}
///*
static void norm_hist(TH1* hist,const std::vector<double> norm_factors){
  double bc(0);
  double nf(0);
  for(size_t i = 0; i < norm_factors.size(); i++){
    nf = norm_factors.at(i);
    bc = hist->GetBinContent(i+1)/(nf > 0 ? nf : 1.);
    // MSG_DEBUG("Bin Content:"<<bc<<" Norm Factor: "<<nf);
    hist->SetBinContent(i+1,bc);
  }
}
//*/
static void paint_hist(TH1* hist, TVirtualPad* pad,
		       size_t pad_pos,size_t n_col, size_t n_row, size_t n_hists){
  set_pad_margins(pad,pad_pos,n_col,n_row);
  if(pad_pos-1 < (n_row - 2)*n_col + n_hists%n_col){
    remove_axis(hist->GetXaxis());
  }
  hist->Draw("H same");
}
void print_2D_slices(std::map<std::string,TTree*> samples,const std::string& plot,
		     TH1* base_hist, const std::string& suffix, 
		     const double target_lumi){
  //This code is (unfortunately) conceptually hairy.  Here's the
  //pseudocode:
  //for each sample
  //    draw 2D jet_e:jet_z
  //        make vector of normalization factors
  //    split jet_e:jet_z into list of hists
  //         draw each hist on a subpad
  //         annotate z bin on each subpad
  //    add sample to legend
  //draw legend
  const size_t n_col = 3;
  const size_t n_bins_z = base_hist->GetNbinsY();
  const size_t n_row = static_cast<size_t>(ceil(n_bins_z/(n_col+0.)));
  TCanvas canv(("2D_stk_canv_"+plot).c_str(),"2D Stack",600*n_col,600*n_row);
  TLatex decorator;
  TLegend* leg = make_legend(0.66,0.01,.33,.25);
  std::map<std::string,aesthetic> hist_styles;
  init_hist_styles(hist_styles);
  std::vector<std::string> sample_names=map_keys(samples);
  decorator.SetTextSize(0.07);
  canv.Divide(n_col,n_row);
  char weight_expr[256];
  snprintf(weight_expr,256,"weight*%.4g",target_lumi);
  size_t pad_pos(0);
  std::vector<double> max_vals(n_bins_z,0.);

  for(std::vector<std::string>::const_iterator n=sample_names.begin();
      n!=sample_names.end(); ++n){
    pad_pos = 1;
    const std::string& name = *n;
    TH2D* HistZvsE = dynamic_cast<TH2D*>(make_normal_hist(base_hist,
							 samples[name],
							 plot, weight_expr,
							 name+"_2D_SLC"));
    std::vector<double> norm_factors = build_norm_factors(HistZvsE);
    TH1D* HistE=NULL;
    for(size_t i=1; i < (size_t)HistZvsE->GetNbinsY()+1; i++){
      HistE=HistZvsE->ProjectionX("_py",i,i+1,"e");
      style_hist(HistE,hist_styles[name]);
      norm_hist(HistE,norm_factors);
      paint_hist(HistE,canv.cd(i), i,n_col,n_row,n_bins_z);
      if(HistE->GetMaximum() > max_vals.at(i-1)){
	max_vals.at(pad_pos-1)=HistE->GetMaximum();
      }
      if(i==1){
	add_to_legend(leg,HistE,hist_styles[name]);
      }
    }
  }

  TAxis* axis = base_hist->GetYaxis();
  std::ostringstream zbl_ss;

  // resize hists so they all fit on the pads
  // also add the z ranges to the upper corner

  for(size_t i=1; i <= n_bins_z; i++){
    TVirtualPad* pad = canv.cd(i);
    zbl_ss.str("");
    zbl_ss << axis->GetBinLowEdge(i)<<" #leq z #leq "<< axis->GetBinLowEdge(i)+axis->GetBinWidth(i);
    // MSG_DEBUG(zbl_ss.str());
    decorator.DrawLatexNDC(0.64,0.9,zbl_ss.str().c_str());
    TIter next(pad->GetListOfPrimitives());
    TH1* h = NULL;
    while((h = dynamic_cast<TH1*>(next()))){
      h->SetMaximum(1.2*max_vals.at(i-1));
    }
  }
  canv.cd(0);
  leg->Draw();
  decorator.SetTextSize(0.04);
  decorator.DrawLatex(0.38,0.01,base_hist->GetTitle());
  std::string outname=plot+suffix;
  replace(outname.begin(),outname.end(),':','_');
  canv.SaveAs(outname.c_str());
}
void print_stack(std::map<std::string,TTree*> samples,const std::string& plot,
		 TH1* base_hist, const std::string& suffix, 
		 const double target_lumi,
		 const char* cut_branches[], const size_t nCuts){
  TCanvas canv(("stk_canv_"+plot).c_str(), "Stack", 600,600);
  TLatex decorator;
  TLegend& leg=*init_legend();

  decorator.SetTextSize(0.04);
  TH1* master = make_normal_hist(base_hist,samples["master"],plot,"weight","_nom");
  master->SetLineWidth(2.);
  master->SetFillStyle(0);
  master->SetLineColor(kBlack);
  const char* cb[]={""};
  THStack* stack = make_stack(base_hist,samples,cut_branches==NULL ? cb : cut_branches,nCuts,plot,leg,target_lumi);
  

  master->Draw("H");
  stack->Draw("HIST same");

  double s_max=stack->GetStack()!=NULL ? ((TH1*)stack->GetStack()->Last())->GetMaximum() : 0.;
  double m_max=master->GetMaximum();
  // MSG_DEBUG("Stack: "<<s_max<<" Master: "<<m_max);
  master->SetMaximum((s_max > m_max ? s_max : m_max)*1.2);
  master->Draw("H same");

  leg.AddEntry(master,"MC12");
  leg.Draw();
  decorator.DrawLatexNDC(0.,0.05,master->GetTitle());
  canv.SaveAs((plot+suffix).c_str());
}
void print_cut_stack(std::map<std::string,TTree*>& samples, 
		     const char* cut_branches[],size_t nCuts, 
		     const std::string& plot, TH1* base_hist, 
		     map<string,string>& CutNames, std::string file_suffix, 
		     double target_lumi){
    TCanvas canv(("canv_"+plot).c_str(),"Cut Plot",1800,800);
  std::map<std::string,aesthetic> hist_styles;
  init_hist_styles(hist_styles);
  TLatex decorator;
  TLegend& leg=*init_legend(0.25,0.72,0.35,0.9);

  decorator.SetTextSize(0.1);
  canv.Divide(3,2);
  canv.SetRightMargin(0);
  canv.SetTopMargin(0);
  THStack* hist = NULL;
  TH1* master=NULL;
  for(size_t i = 0; i < nCuts; i++){
    set_pad_margins(canv.cd(i+1),i+1,nCuts);
    master = make_normal_hist(base_hist,samples["master"],cut_branches, i ,plot);
    hist = make_stack(base_hist,samples, cut_branches, i,plot, leg, target_lumi);
    hist->Draw("HIST");
    master->Draw("H same");
    if( i < 3){ //top row
      remove_axis(hist->GetXaxis());
    }
    decorator.DrawLatexNDC(0.5,0.75,CutNames[cut_branches[i]].c_str());
  }
  // canv.cd(1);
  // canv.cd(nCuts);
  canv.cd(0);
  leg.Clear();
  TIter next(hist->GetHists());
  TH1* h=NULL;
  while((h = (TH1*)next())) {
    add_to_legend(&leg,h,hist_styles[string(h->GetName()).substr(0,5)]);
  }
  leg.AddEntry(master,"MC12");
  leg.Draw();
  decorator.SetTextSize(0.04);
  decorator.DrawLatex(0.0,0.05,base_hist->GetTitle());
  canv.SaveAs((plot+file_suffix).c_str());
}
