#include <string>
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
    TTree* const tree = samples[name];
    TH1* hist =(TH1*)base_hist->Clone((name+plot+"_"+cut_branches[cut_index]).c_str());
    hist_list[i]=hist;
    cut_expr=((cut_index == 0) ? "weight" : "weight*" 
	      + str_join("*",cut_branches,0,cut_index+1));
    cut_expr+="%.4g";
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
  TH1* master = make_normal_hist(base_hist,samples["master"],plot);
  master->SetLineWidth(2.);
  master->SetFillStyle(0);
  master->SetLineColor(kBlack);
  const char* cb[]={""};
  THStack* stack = make_stack(base_hist,samples,cut_branches==NULL ? cb : cut_branches,nCuts,plot,leg,target_lumi);
  

  master->Draw("H");
  //canv.SetLogY();
  stack->Draw("HIST same");
  double s_max=((TH1*)stack->GetStack()->Last())->GetMaximum();
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
    set_pad_margins(canv.cd(i+1),i+1,true);
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
