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
#include "math.hh"
#include "color.hh"

using std::string;
using std::map;

void scale_stack(TH1** hist_list, const size_t n, double sf){
  for(size_t i=0; i < n; i++){
    hist_list[i]->Scale(sf);
  }
}
THStack* make_stack(TH1* base_hist, std::map<std::string,TTree*>& samples,
		    const std::vector<std::string>& cut_branches, int cut_index, 
		    const std::string& plot, TLegend& leg, double target_lumi ){
  std::vector<std::string> sample_names = map_keys(samples);
  std::map<std::string,aesthetic> hist_styles;
  init_hist_styles(hist_styles);
  THStack* stack = new THStack(("stack_"+plot).c_str(),base_hist->GetTitle());
  double total=0.;
  size_t num_hists=sample_names.size();
  std::vector<int> colors=qualitative(DYNAMIC,num_hists);
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
    std::string suffix = cut_index == 0 ? "" : str_join("_",cut_branches,0,cut_index+1);
    // MSG_DEBUG(suffix);
    if(suffix.find("(")!=std::string::npos){
      suffix.replace(suffix.find("("),suffix.rfind(")"),"signal_region");
    } 
    TH1* hist =(TH1*)base_hist->Clone((name+plot+"_"+ suffix).c_str());
    // MSG_DEBUG(hist->GetName());
    hist_list[i]=hist;
    cut_expr=((cut_index == 0) ? "SF*weight" : "SF*weight*" 
	      + str_join("*",cut_branches,0,cut_index+1));
    cut_expr+="*%.4g";
    snprintf(cut_str,sizeof(cut_str)/sizeof(*cut_str),cut_expr.c_str(),target_lumi);
    draw_histo(tree,plot.c_str(),hist->GetName(), cut_str);
    total+=hist->Integral();
    stack->Add(hist);
    aesthetic style;
    if(hist_styles.find(name)!=hist_styles.end()){
      style=hist_styles[name];
    }
    else{
      //MSG_DEBUG(name<<" is not found in style list, making new on on the fly.")
      style=hist_aes(name.c_str(),colors[i],1001,kSolid);
      MSG_DEBUG(style.leg_label)
    }
    style_hist(hist,style);
    add_to_legend(&leg,hist,style);
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
  snprintf(weight_expr,256,"SF*weight*%.4g",target_lumi);
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
  add_atlas_badge(canv,0.66,0.25,target_lumi);
  decorator.SetTextSize(0.04);
  decorator.DrawLatex(0.01,0.02,base_hist->GetTitle());
  std::string outname=plot+suffix;
  replace(outname.begin(),outname.end(),':','_');
  canv.SaveAs(outname.c_str());
}
double norm_stack(THStack& stack){
  TIter next(stack.GetHists());
  TH1* h = NULL;
  double max(0);
  while((h = dynamic_cast<TH1*>(next()))){
    h->Scale(1/h->Integral());
    if(h->GetMaximum() > max){
      max=h->GetMaximum();
    }
  }
  return max;
}
num_err integral_error(TH1D* hist){
  num_err result;
  num_err bin;
  for(size_t i=1; i < size_t(hist->GetNbinsX())+1; i++){
    bin.val=hist->GetBinContent(i);
    bin.err=hist->GetBinError(i);
    result=add(result,bin);
  }
  return result;
}
static std::vector<num_err> build_norm_factors( const TH2D *HistZvsE){
  std::vector<num_err> result;
  result.reserve(HistZvsE->GetNbinsX());
  TH1D* hist=NULL;
  char name[256];
  
  for(size_t i = 1; i < (size_t)HistZvsE->GetNbinsX()+1; i++){
    hist=HistZvsE->ProjectionY(name,i,i+1,"e");
    snprintf(name,LEN(name),"%s_%zu_px",hist->GetName(),i);
    result.push_back(integral_error(hist));
  }
  return result;
}
///*
static void norm_hist(TH1* hist,const std::vector<num_err> norm_factors){
  num_err bc;
  num_err nf;
  // MSG_DEBUG(hist->GetName());
  for(size_t i = 0; i < norm_factors.size(); i++){
    nf = norm_factors.at(i);
    if (nf.val == 0){
      nf.val=1;
      nf.err=0;
    }
    bc.val = hist->GetBinContent(i+1);///(nf > 0 ? nf : 1.);
    bc.err = hist->GetBinError(i+1);
    // MSG_DEBUG("Bin Content:"<<str_rep(bc));
    bc=div(bc,nf);
    // MSG_DEBUG("Bin Content:"<<str_rep(bc)<<" Norm Factor: "<<str_rep(nf));
    hist->SetBinContent(i+1,bc.val);
    hist->SetBinError(i+1,0);
  }
}
//*/
/*
static void paint_hist(TH1* hist, TVirtualPad* pad,
		       size_t pad_pos,size_t n_col, size_t n_row, size_t n_hists){
  set_pad_margins(pad,pad_pos,n_col,n_row);
  if(pad_pos-1 < (n_row - 2)*n_col + n_hists%n_col){
    remove_axis(hist->GetXaxis());
  }
  hist->Draw("H same");
}
*/
void print_2D_slices(std::map<std::string,TTree*> samples,const std::string& plot,
		     TH1* base_hist, const std::string& suffix, 
		     const double target_lumi, bool norm){
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
  const size_t n_bins_y = base_hist->GetNbinsY();
  const size_t n_row = static_cast<size_t>(ceil(n_bins_y/(n_col+0.)));
  TCanvas canv(("2D_stk_canv_"+plot).c_str(),"2D Stack",600*n_col,600*n_row);
  TLatex decorator;
  TLegend* leg = init_legend(0.66,0.01,.33,.25);
  std::map<std::string,aesthetic> hist_styles;
  init_hist_styles(hist_styles);
  std::vector<std::string> sample_names=map_keys(samples);
  decorator.SetTextSize(0.07);
  canv.Divide(n_col,n_row);
  char weight_expr[256];
  snprintf(weight_expr,256,"SF*weight*%.4g",target_lumi);
  std::vector<double> max_vals(n_bins_y,0.);

  for(std::vector<std::string>::const_iterator n=sample_names.begin();
      n!=sample_names.end(); ++n){
    const std::string& name = *n;
    TH2D* Hist2D = dynamic_cast<TH2D*>(make_normal_hist(base_hist,
							 samples[name],
							 plot, weight_expr,
							 name+"_2D_SLC"));
    std::vector<num_err> norm_factors;
    if (norm){
      norm_factors= build_norm_factors(Hist2D);
    }
    TH1D* HistX=NULL;
    char prj_name[256];
    for(size_t i=1; i < (size_t)Hist2D->GetNbinsY()+1; i++){
      canv.cd(i);
      snprintf(prj_name,LEN(prj_name),"%s_%zu_py",Hist2D->GetName(),i);
      HistX=Hist2D->ProjectionX(prj_name,i,i+1,"e");
      // MSG_DEBUG(i<<" "<<HistX<<" "<<HistX->GetName()<<" "<<HistX->GetEntries());
      style_hist(HistX,hist_styles[name]);
      HistX->SetFillStyle(0);
      if(norm){
	norm_hist(HistX,norm_factors);
      }
      canv.cd(i);
      HistX->Draw("H same");
      // paint_hist(HistX,canv.cd(i), i,n_col,n_row,n_bins_y);
      if(HistX->GetMaximum() > max_vals.at(i-1)){
      	max_vals.at(i-1)=HistX->GetMaximum();
      }
      if(i==1){
	add_to_legend(leg,HistX,hist_styles[name]);
      }
    }
  }

  TAxis* axis = base_hist->GetYaxis();
  std::ostringstream zbl_ss;
  // resize hists so they all fit on the pads
  // also add the z ranges to the upper corner
  for(size_t i=1; i <= n_bins_y; i++){
    TVirtualPad* pad = canv.cd(i);
    zbl_ss.str("");
    zbl_ss << axis->GetBinLowEdge(i)<<" #leq "<<axis->GetTitle()<<" #leq "<< axis->GetBinLowEdge(i)+axis->GetBinWidth(i);
    // MSG_DEBUG(zbl_ss.str());
    decorator.DrawLatexNDC(0.2,0.85,zbl_ss.str().c_str());
    TIter next(pad->GetListOfPrimitives());
    TH1* h = NULL;
    while((h = dynamic_cast<TH1*>(next()))){
      h->SetMaximum(1.2*max_vals.at(i-1));
    }
  }

  canv.cd(0);
  // this damn thing is too big
  add_atlas_badge(canv,0.4,0.1,target_lumi);
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
		 const std::vector<std::string>& cut_branches){
  TCanvas canv(("stk_canv_"+plot).c_str(), "Stack", 600,600);
  if(plot.find("pt")!=std::string::npos){
    canv.SetLogy(true);
  }
  TLatex decorator;
  TLegend& leg=*init_legend();
  decorator.SetTextSize(0.04);
  std::vector<std::string> extended_cbs(cut_branches);
  extended_cbs.push_back("((2.904 < jpsi_m && jpsi_m < 3.29) && (-1 < jpsi_tau && jpsi_tau < 0.25))");
  size_t cut_idx=extended_cbs.size()-1;
  TH1* master = make_normal_hist(base_hist,samples["master"], extended_cbs, cut_idx, plot);
  master->SetLineWidth(2.);
  master->SetFillStyle(0);
  master->SetLineColor(kBlack);
  THStack* stack = make_stack(base_hist,samples,extended_cbs, cut_idx, plot, leg, target_lumi);
  master->Draw("H");
  stack->Draw("H same");
  double s_max=stack->GetStack()!=NULL ? ((TH1*)stack->GetStack()->Last())->GetMaximum() : 0.;
  double m_max=master->GetMaximum();
  master->SetMaximum((s_max > m_max ? s_max : m_max)*1.2);
  master->Draw("H same");
  leg.AddEntry(master,"MC12");
  leg.Draw();
  decorator.DrawLatexNDC(0.,0.05,master->GetTitle());
  add_atlas_badge(canv,0.2,0.9,target_lumi);
  canv.SaveAs((plot+suffix).c_str());
}
void print_cut_stack(std::map<std::string,TTree*>& samples, 
		     const std::vector<std::string>& cut_branches,
		     const std::string& plot, TH1* base_hist, 
		     std::map<std::string,std::string>& CutNames, 
		     const std::string& file_suffix, const double target_lumi){
  // include an extra frame for the legend and atlas boilerplate
  const size_t n_frame = cut_branches.size()+1;
  const size_t n_col = 3;
  const size_t n_row = static_cast<size_t>(ceil(n_frame/(n_col+0.)));
  TCanvas canv(("2D_stk_canv_"+plot).c_str(),"2D Stack",600*n_col,600*n_row);
  std::map<std::string,aesthetic> hist_styles;
  init_hist_styles(hist_styles);
  TLatex decorator;
  TLegend& leg=*init_legend(0.25,0.72,0.35,0.9);
  decorator.SetTextSize(0.1);
  canv.Divide(n_row,n_col);
  canv.SetRightMargin(0);
  canv.SetTopMargin(0);
  THStack* hist = NULL;
  TH1* master=NULL;
  size_t nCuts=cut_branches.size();
  for(size_t i = 1; i < nCuts+1; i++){
    std::vector<std::string> extended_cbs(cut_branches.begin(),cut_branches.begin()+i);
    extended_cbs.push_back("((2.904 < jpsi_m && jpsi_m < 3.29) && (-1 < jpsi_tau && jpsi_tau < 0.25))");
    size_t cut_idx=extended_cbs.size()-1;
    TVirtualPad* pad = canv.cd(i);
    set_pad_margins(pad,i+1,nCuts);
    if(plot.find("pt")!=std::string::npos ||
       plot.find("jpsi_tau")!=std::string::npos){
      pad->SetLogy(true);
    }
      
    master = make_normal_hist(base_hist,samples["master"], extended_cbs, cut_idx ,plot);
    hist = make_stack(base_hist,samples, extended_cbs, cut_idx,plot, leg, target_lumi);
    master->Draw("e1");
    if(hist){
      hist->Draw("H same");
    }
    // MSG_DEBUG(hist->GetName());
    MSG_DEBUG(cut_branches[i-1]);
    decorator.DrawLatexNDC(0.5,0.75,CutNames[cut_branches[i-1]].c_str());
  }
  canv.cd(0);
  leg.Clear();
  leg.AddEntry(master,"Data");
  leg.Draw();
  decorator.SetTextSize(0.04);
  decorator.DrawLatex(0.0,0.05,base_hist->GetTitle());
  // add_atlas_badge(canv,0.2,0.9,target_lumi);
  canv.SaveAs((plot+file_suffix).c_str());
}
