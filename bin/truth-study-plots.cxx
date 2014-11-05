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
void print_stack(std::map<std::string,TTree*> samples,const std::string& plot,
		 TH1* base_hist, const std::string& suffix){
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

  
  TCanvas canv(("stk_canv_"+plot).c_str(), "Stack", 600,600);
  TLatex decorator;
  TLegend leg(0.75,0.68,0.99,0.92);
  //leg.SetNColumns(3);
  leg.SetFillColor(0);
  leg.SetFillStyle(0);
  leg.SetBorderSize(0);

  decorator.SetTextSize(0.04);
  THStack stack(("stack_"+plot).c_str(),base_hist->GetTitle());
  TH1* master = make_normal_hist(base_hist,samples["master"],plot);
  for(size_t i=0; i < sizeof(sample_names)/sizeof(*sample_names); i++){
    const string name(sample_names[i]);
    TTree* const tree = samples[name];
    TH1* hist = (TH1*)base_hist->Clone((plot+name+"_NOM").c_str());
    draw_histo(tree,plot.c_str(),hist->GetName(),"");
    leg.AddEntry(hist,leg_map[name].c_str(),"f");
    hist->SetLineColor(color_map[name]);
    hist->SetFillColor(color_map[name]);
    stack.Add(hist);
  }
  stack.Draw("HIST ");
  master->SetLineWidth(2.);
  master->SetFillStyle(0);
  master->SetLineColor(kBlack);
  master->Draw("H same");
  leg.Draw();
  decorator.DrawLatexNDC(0.,0.05,master->GetTitle());
  canv.SaveAs((plot+suffix).c_str());
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

  map<string,TTree*> sample_trees;
  sample_trees["master"]=retrieve<TTree>(argv[1],"mini");
  const char* sample_names[]={"1S0_8","3S1_8","3PJ_8","3S1_1","3PJ_1"};
  char fname[256];
  for(size_t i=0; i < sizeof(sample_names)/sizeof(*sample_names); i++){
    snprintf(fname,256,"%s.mini.root",sample_names[i]);
    sample_trees[sample_names[i]]=retrieve<TTree>(fname,"mini");
  }
  map<string,TH1D*> HistBook;
  HistBook["jet_pt"]=new TH1D("jet_pt","Jet p_{T};p_{T} [GeV];evts/binwidth",50,0,250);
  HistBook["jet_eta"]=new TH1D("jet_eta","Jet #eta;#eta;evts/binwidth",50,-2.6,2.6);
  HistBook["jet_e"]=new TH1D("jet_e","Jet E;E [GeV]; evts/binwidth",50,0,500);
  HistBook["jet_z"]=new TH1D("jet_z","Jet Z;z;evts/binwidth",50,0,1.);
  HistBook["tau1"]=new TH1D("tau1","N Subjettiness #tau_{1};#tau_{1};evts/binwidth",100,0,1.);
  HistBook["tau2"]=new TH1D("tau2","N Subjettiness #tau_{2};#tau_{2};evts/binwidth",100,0,1.);
  HistBook["tau3"]=new TH1D("tau3","N Subjettiness #tau_{3};#tau_{3};evts/binwidth",100,0,1.);
  HistBook["tau32"]=new TH1D("tau32","N Subjettiness #tau_{32};#tau_{32};evts/binwidth",100,0,1.2);
  HistBook["tau21"]=new TH1D("tau21","N Subjettiness #tau_{21};#tau_{21};evts/binwidth",100,0,1.2);
  HistBook["delta_r"]=new TH1D("delta_r","#Delta R(J/#psi,Jet); #Delta R; evts/binwidth",50,0,1.);
  HistBook["jpsi_pt"]=new TH1D("jpsi_pt","J/#psi p_{T};p_{T} [GeV];evts/binwidth",50,0,200);
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
    print_stack(sample_trees,plot,HistBook[plot],"_stack.pdf");
  }
  return 0;
}
