#include <string>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <map>

#include "TLegend.h"
#include "TLatex.h"
#include "THStack.h"
#include "TColor.h"
#include "TList.h"
#include "TTree.h"
#include "TCanvas.h"

#include "stack-utils.hh"
#include "root-sugar.hh"
#include "histo-utils.hh"
#include "histo-meta-data.hh"

using std::string;
using std::map;

void scale_stack(TH1** hist_list, const size_t n, double sf){
  for(size_t i=0; i < n; i++){
    hist_list[i]->Scale(sf);
  }
}
void style_hist(TH1* hist, TLegend* leg, const int color, const char* leg_name){
  leg->AddEntry(hist,leg_name,"f");
  hist->SetLineColor(color);
  hist->SetFillColor(color);
}
THStack* make_stack(TH1* base_hist, std::map<std::string,TTree*>& samples,
		    const char* cut_branches[], int cut_index, 
		    const std::string& plot, TLegend& leg, double target_lumi ){
  std::vector<std::string> sample_names = map_keys(samples);
  //const char* sample_names[]={"3PJ_8","3S1_8","1S0_8","3PJ_1","3S1_1"};
  std::map<std::string,int> color_map;
  init_colors(color_map);
  std::map<std::string,std::string> leg_map;
  init_leg_names(leg_map);
  THStack* stack = new THStack(("stack_"+plot).c_str(),base_hist->GetTitle());
  double total=0.;
  size_t num_hists=sample_names.size();//sizeof(sample_names)/sizeof(*sample_names);
  TH1** hist_list = (TH1**)calloc(num_hists, sizeof(TH1*));
  string cut_expr;
  char cut_str[1024];
  for(size_t i=0; i < num_hists; i++){
    cut_expr.clear();
    const string name(sample_names[i]);
    if(name=="master"){
      continue;
    }
    // MSG_DEBUG("Processing: "<<name);
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
    style_hist(hist,&leg,color_map[name],leg_map[name].c_str());
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
  std::map<std::string,std::string> leg_map;
  init_leg_names(leg_map);
  std::vector<std::string> sample_names=map_keys(samples);
  decorator.SetTextSize(0.07);
  canv.Divide(n_col,n_row);
  // canv.cd(1);
  char weight_expr[256];
  snprintf(weight_expr,256,"weight*%.4g",target_lumi);
  Double_t Red[3]    = { 1.00, 0.00, 0.00};
  Double_t Green[3]  = { 0.00, 1.00, 0.00};
  Double_t Blue[3]   = { 0.00, 0.00, 1.00};
  Double_t Length[3] = { 0.00, 0.50, 1.00 };
  Int_t nb=100;
  TColor::CreateGradientColorTable(3,Length,Red,Green,Blue,nb);
  
  for(size_t i=0; i < n_samp; i++){
    set_pad_margins(canv.cd(i+1),i+1,n_samp,n_col,n_row,false);
    std::string& name = sample_names[i];
    TH1* hist = make_normal_hist(base_hist,samples[name],plot,
				 weight_expr,name+"_2D_STK");
    canv.cd(i+1);
    hist->SetContour(nb);
    hist->Draw("COL");
    if(i < (n_row - 2)*n_col + n_samp%n_col){
      remove_axis(hist->GetXaxis());
    }
    if(i%n_col!=0){
      remove_axis(hist->GetYaxis());
    }

    decorator.DrawLatexNDC(0.45,0.85,leg_map[name].c_str());
  }
  canv.cd(0);
  decorator.SetTextSize(0.04);
  decorator.DrawLatex(0.0,0.05,base_hist->GetTitle());
  std::string outname=plot+suffix;
  replace(outname.begin(),outname.end(),':','_');
  canv.SaveAs(outname.c_str());
}
/*
static std::string transpose(std::string base,std::string anchor){
  // C-M-t in emacs... 
  std::string result;
  size_t anchor_pos = base.find(anchor);
  if(anchor_pos==std::string::npos){
    return base;
  }
  result = base.substr(anchor_pos+1) + anchor + base.substr(0,anchor_pos);
  return result;
}
*/

static std::vector<double> build_norm_factors(TH1* HistZvsE){
  THStack stack(HistZvsE,"y");
  std::vector<double> result;
  result.reserve(HistZvsE->GetNbinsX());
  TIter next(stack.GetHists());
  TH1* hist = NULL;
  while((hist = dynamic_cast<TH1*>(next()))){
    result.push_back(hist->Integral());
  }
  return result;
}
static void norm_hist(TH1* hist,const std::vector<double> norm_factors){
  double bc(0);
  double nf(0);
  for(size_t i = 0; i < norm_factors.size(); i++){
    nf = norm_factors.at(i);
    bc = hist->GetBinContent(i)/(nf > 0 ? nf : 1.);
    hist->SetBinContent(i,bc/norm_factors.at(i));
  }
}

static void paint_hist(TH1* hist, TVirtualPad* pad,
		       size_t pad_pos,size_t n_col, size_t n_row, size_t n_hists,
		       int color){
  set_pad_margins(pad,pad_pos,n_col,n_row);
  if(pad_pos-1 < (n_row - 2)*n_col + n_hists%n_col){
    remove_axis(hist->GetXaxis());
  }

  hist->SetLineColor(color);
  hist->SetFillStyle(0);
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
  std::map<std::string,int> color_map;
  init_colors(color_map);
  std::map<std::string,std::string> leg_map;
  init_leg_names(leg_map);
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
    TH1* HistZvsE = make_normal_hist(base_hist,samples[name],plot,
				     weight_expr,name+"_2D_SLC");
    std::vector<double> norm_factors = build_norm_factors(HistZvsE);
    // norm_hist(HistZvsE,norm_factors);
    THStack proj_stack(HistZvsE,"x");
    TH1* HistE = NULL;
    TIter next(proj_stack.GetHists());
    while((HistE = dynamic_cast<TH1*>(next()))){
      paint_hist(HistE,canv.cd(pad_pos),
		 pad_pos,n_col,n_row, n_bins_z,
		 color_map[name]);
      if(HistE->GetMaximum() > max_vals.at(pad_pos-1)){
	max_vals.at(pad_pos-1) = HistE->GetMaximum();
      }
      if(pad_pos==1){
	leg->AddEntry(HistE,leg_map[name].c_str(),"l");
      }
      pad_pos++;
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
    MSG_DEBUG(zbl_ss.str());
    decorator.DrawLatexNDC(0.64,0.9,zbl_ss.str().c_str());
    TIter next(pad->GetListOfPrimitives());
    TH1* h = NULL;
    while((h = dynamic_cast<TH1*>(next()))){
      h->SetMaximum(1.2*max_vals.at(i-1));
    }
    h=dynamic_cast<TH1*>(pad->GetListOfPrimitives()->First());
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
  TLegend leg(0.75,0.68,0.99,0.92);
  leg.SetFillColor(0);
  leg.SetFillStyle(0);
  leg.SetBorderSize(0);

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
  std::map<std::string,std::string> leg_map;
  init_leg_names(leg_map);
  TLatex decorator;
  TLegend leg(0.25,0.72,0.35,0.9);
  leg.SetFillColor(0);
  leg.SetFillStyle(0);
  leg.SetBorderSize(0);

  decorator.SetTextSize(0.1);
  canv.Divide(3,2);
  canv.SetRightMargin(0);
  canv.SetTopMargin(0);
  THStack* hist = NULL;
  TH1* master=NULL;
  for(size_t i = 0; i < nCuts; i++){
    set_pad_margins(canv.cd(i+1),i+1,nCuts);
    master = make_normal_hist(base_hist,samples["master"],cut_branches, i ,plot);
    master->SetLineWidth(1.5);
    master->SetFillStyle(0);
    master->SetLineColor(kBlack);
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
    leg.AddEntry(h,leg_map[string(h->GetName()).substr(0,5)].c_str(),"f");
  }
  leg.AddEntry(master,"MC12");
  leg.Draw();
  decorator.SetTextSize(0.04);
  decorator.DrawLatex(0.0,0.05,base_hist->GetTitle());
  canv.SaveAs((plot+file_suffix).c_str());
}
