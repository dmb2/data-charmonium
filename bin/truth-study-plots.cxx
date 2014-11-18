#include <cmath>
#include <cstdlib>
#include <iostream>
#include <map>
#include <string>

#include "TFile.h"
#include "TTree.h"
#include "TCanvas.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TStyle.h"
#include "TLegend.h"
#include "TLatex.h"
#include "THStack.h"
#include "TColor.h"
#include "TROOT.h"

#include "AtlasStyle.hh"
#include "histo-utils.hh"
#include "root-sugar.hh"

using namespace std;
void usage(const char* name){
  cout <<"Usage: "<< name << " input_file.root"<<endl;
}
/*
void scale_stack(THStack* stack, double num){
  double scale_factor = num/((TH1*)stack->GetStack()->Last())->Integral();
  if(!isnormal(scale_factor) || scale_factor <= 0){
    return;
  }
  TIter next(stack->GetHists());
  TH1* hist=NULL;
  while((hist = (TH1*)next())) {
    hist->Scale(scale_factor);
  }
}
*/
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
		    const std::string& plot, TLegend& leg, double n_master){
  const char* sample_names[]={"3PJ_8","3S1_8","1S0_8","3PJ_1","3S1_1"};
  std::map<std::string,int> color_map;
  // Octet colors
  color_map["1S0_8"]=TColor::GetColor(222,235,247);
  color_map["3S1_8"]=TColor::GetColor(158,202,225);
  color_map["3PJ_8"]=TColor::GetColor(49,130,189);
  // Singlet colors
  color_map["3S1_1"]=TColor::GetColor(252,146,114);
  color_map["3PJ_1"]=TColor::GetColor(222,45,38);
  std::map<std::string,std::string> leg_map;
  leg_map["1S0_8"]="^{1}S^{(8)}_{0}";
  leg_map["3S1_1"]="^{3}S^{(1)}_{1}";
  leg_map["3S1_8"]="^{3}S^{(1)}_{8}";
  leg_map["3PJ_1"]="^{3}P^{(1)}_{J}";
  leg_map["3PJ_8"]="^{3}P^{(8)}_{J}";
  THStack* stack = new THStack(("stack_"+plot).c_str(),base_hist->GetTitle());
  double total=0.;
  size_t num_hists=sizeof(sample_names)/sizeof(*sample_names);
  TH1** hist_list = (TH1**)calloc(num_hists, sizeof(TH1*));
  for(size_t i=0; i < num_hists; i++){
    const string name(sample_names[i]);
    TTree* const tree = samples[name];
    TH1* hist =(TH1*)base_hist->Clone((name+plot+"_"+cut_branches[cut_index]).c_str());
    hist_list[i]=hist;
    draw_histo(tree,plot.c_str(),hist->GetName(),
	       str_join("*",cut_branches,0,cut_index+1).c_str());
    total+=hist->Integral();
    stack->Add(hist);
    style_hist(hist,&leg,color_map[name],leg_map[name].c_str());
  }
  //ROOT SUCKS
  scale_stack(hist_list,num_hists,n_master > 0 ? n_master/total : 1);
  free(hist_list);
  return stack;
}

void print_stack(std::map<std::string,TTree*> samples,const std::string& plot,
		 TH1* base_hist, const std::string& suffix){
  
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
  THStack* stack = make_stack(base_hist,samples,cb,0,plot,leg,master->Integral());
  
  stack->Draw("HIST ");
  master->Draw("H same");
  leg.AddEntry(master,"MC12");
  leg.Draw();
  decorator.DrawLatexNDC(0.,0.05,master->GetTitle());
  canv.SaveAs((plot+suffix).c_str());
}
void print_cut_stack(std::map<std::string,TTree*>& samples, 
		     const char* cut_branches[],size_t nCuts, 
		     const std::string& plot, TH1* base_hist, 
		     map<string,string>& CutNames, std::string file_suffix){
    TCanvas canv(("canv_"+plot).c_str(),"Cut Plot",1800,800);
  std::map<std::string,std::string> leg_map;
  leg_map["1S0_8"]="^{1}S^{(8)}_{0}";
  leg_map["3S1_1"]="^{3}S^{(1)}_{1}";
  leg_map["3S1_8"]="^{3}S^{(1)}_{8}";
  leg_map["3PJ_1"]="^{3}P^{(1)}_{J}";
  leg_map["3PJ_8"]="^{3}P^{(8)}_{J}";
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
    hist = make_stack(base_hist,samples, cut_branches, i,plot, leg, master->Integral());
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
int main(const int argc, const char* argv[]){
  if(argc!=2){
    usage(argv[0]);
    return 0;
  }
  AtlasStyle style;
  style.SetAtlasStyle();
  gStyle->SetFrameLineWidth(0.0);
  // gStyle->SetPadTickX(0);
  // gStyle->SetPadTickY(0);
  const char* cut_branches[]={"num_jets_p", "jpsi_pt_p",    
			      "jpsi_eta_p", "delta_r_p",    
			      "jet_eta_p", "jet_pt_p"};
  size_t nCuts=sizeof(cut_branches)/sizeof(*cut_branches);
  map<string,string> pretty_cNames;
  pretty_cNames["num_jets_p"]="N_{j} #geq 1"; 
  pretty_cNames["jpsi_pt_p"]="p_{T}(J/#psi) > 20 GeV";    
  pretty_cNames["jpsi_eta_p"]="|#eta(J/#psi)| < 2.5";
  pretty_cNames["delta_r_p"]="#Delta R(Jet;J/#psi) < 0.4";    
  pretty_cNames["jet_eta_p"]="|#eta(jet)| < 2.5"; 
  pretty_cNames["jet_pt_p"]="p_{T}(jet) > 45 GeV";
  pretty_cNames["mu_trigger_p"]="1 #mu Trigger, 2012";

  map<string,TTree*> sample_trees;
  sample_trees["master"]=retrieve<TTree>(argv[1],"mini");
  const char* sample_names[]={"1S0_8","3S1_8","3PJ_8","3S1_1","3PJ_1"};
  char fname[256];
  for(size_t i=0; i < sizeof(sample_names)/sizeof(*sample_names); i++){
    snprintf(fname,256,"%s.mini.root",sample_names[i]);
    sample_trees[sample_names[i]]=retrieve<TTree>(fname,"mini");
  }
  map<string,TH1D*> HistBook;
  HistBook["jet_pt"]=new TH1D("jet_pt","Jet p_{T};p_{T} [GeV];evts/binwidth",50,0,400);
  HistBook["jet_eta"]=new TH1D("jet_eta","Jet #eta;#eta;evts/binwidth",50,-2.6,2.6);
  HistBook["jet_e"]=new TH1D("jet_e","Jet E;E [GeV]; evts/binwidth",50,0,500);
  HistBook["jet_z"]=new TH1D("jet_z","Jet Z;z;evts/binwidth",50,0,1.);
  //cout<<"SWITCH NSUBJETTINESS AXES RANGE BACK TO [0.,1.]"<<endl;
  HistBook["tau1"]=new TH1D("tau1","N Subjettiness #tau_{1};#tau_{1};evts/binwidth",100,0,1.);
  HistBook["tau2"]=new TH1D("tau2","N Subjettiness #tau_{2};#tau_{2};evts/binwidth",100,0,1.);
  HistBook["tau3"]=new TH1D("tau3","N Subjettiness #tau_{3};#tau_{3};evts/binwidth",100,0,1.);
  HistBook["tau32"]=new TH1D("tau32","N Subjettiness #tau_{32};#tau_{32};evts/binwidth",100,0,1.2);
  HistBook["tau21"]=new TH1D("tau21","N Subjettiness #tau_{21};#tau_{21};evts/binwidth",100,0,1.2);
  HistBook["delta_r"]=new TH1D("delta_r","#Delta R(J/#psi,Jet); #Delta R; evts/binwidth",50,0,1.);
  HistBook["jpsi_pt"]=new TH1D("jpsi_pt","J/#psi p_{T};p_{T} [GeV];evts/binwidth",50,0,300);
  HistBook["jpsi_eta"]=new TH1D("jpsi_eta","J/#psi #eta;#eta;evts/binwidth",50,-2.6,2.6);
  HistBook["jpsi_e"]=new TH1D("jpsi_e","J/#psi E;E [GeV]; evts/binwidth",50,0,600);
  vector<string> plots = map_keys(HistBook);
  for(map<string,TH1D*>::iterator item=HistBook.begin(); 
      item != HistBook.end(); ++item){
    TH1D* hist = item->second;
    hist->Sumw2();
    hist->SetMarkerStyle(1);
    hist->SetLineWidth(1.);
    hist->SetFillStyle(1001);
    hist->SetDrawOption("H");
  }
  for(vector<string>::const_iterator p=plots.begin(); p!=plots.end(); ++p){
    const std::string& plot = *p;
    // print_stack(sample_trees,plot,HistBook[plot],"_stack.pdf");
    print_cut_stack(sample_trees,cut_branches,nCuts,plot,
		    HistBook[plot],pretty_cNames,
		    "_normal.pdf");
  }
  return 0;
}
