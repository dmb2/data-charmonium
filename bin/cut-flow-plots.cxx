#include <iostream>
#include <map>
#include <string>

#include "TFile.h"
#include "TTree.h"
#include "TCanvas.h"
#include "TH1D.h"
#include "TStyle.h"
#include "TLatex.h"
#include "TPaveText.h"

#include "AtlasStyle.hh"

using namespace std;
template<class T1,class T2>
inline vector<T1> map_keys(map<T1,T2>& inputMap){
  vector<T1> keys;
  for(typename map<T1,T2>::const_iterator i=inputMap.begin();
      i!=inputMap.end(); ++i){
    keys.push_back(i->first);
  }
  return keys;
}
template<class T1,class T2>
inline vector<T2> map_values(map<T1,T2>& inputMap){
  vector<T2> values;
  for (typename map<T1,T2>::const_iterator i=inputMap.begin();
       i!=inputMap.end(); ++i){
    values.push_back(i->second);
  }
  return values;
}

inline vector<string> add_prefix(string prefix, vector<string> strings){
  vector<string> result;
  result.reserve(strings.size());
  for(vector<string>::const_iterator str=strings.begin(); str!=strings.end(); ++str){
    result.push_back(prefix + *str);
  }
  return result;
}
string str_join(string base, const char* strings[],size_t start, size_t end){
  string result(strings[start]);
  if(start==end){
    return "";
  }
  for(size_t i=(start+1); i < end; i++){
    result+=(base + string(strings[i]));
  }
  return result;
}
void draw_histo(TTree* tree,const char* branch_name, const char* hist_name, const char* cut_expr){
  char branch_expr[200];
  snprintf(branch_expr,200,"%s>>%s", branch_name,hist_name);
  tree->Draw(branch_expr,cut_expr,"goff");
}
void remove_axis(TAxis* axis){
  axis->SetLabelSize(0);
  axis->SetTitle("");
  axis->SetLabelOffset(999);
}
void set_pad_margins(TVirtualPad* pad,int pad_pos){
  pad->SetRightMargin(0);
  pad->SetTopMargin(0);
  if (pad_pos < 4){
    pad->SetBottomMargin(0);
  }
  if(pad_pos != 4 && pad_pos != 1){
    pad->SetLeftMargin(0);
  }
}
void make_ratio_hist(TH1D* ratio, TTree* tree,const char* cut_names[],size_t cut_index,string plot){
  string hNDenom = plot+str_join("_",cut_names,0,cut_index)+"_tmp";
  TH1D* hDenom = (TH1D*)ratio->Clone(hNDenom.c_str());
  draw_histo(tree, plot.c_str(), hNDenom.c_str(),
	     str_join("*", cut_names, 0,cut_index).c_str());
  draw_histo(tree, plot.c_str(), ratio->GetName(),
	     str_join("*", cut_names, 0,cut_index+1).c_str());
  ratio->Divide(hDenom);
  ratio->SetMaximum(1.2);
  ratio->SetMinimum(0.);
}
int main(const int argc, const char* argv[]){
  gStyle->SetOptStat(0);
  AtlasStyle style;
  style.SetAtlasStyle();
  gStyle->SetFrameLineWidth(0.5);
  vector<string> args;
  for(int i=0; i < argc; i++){
    args.push_back(string(argv[i]));
  }
  TFile InFile("cut_tree.root");
  TTree* CutTree = dynamic_cast<TTree*>(InFile.Get("mini"));
  const char* cut_branches[]={"num_jets_p", "jpsi_pt_p",    
			      "jpsi_eta_p", "delta_r_p",    
			      "jet_eta_p", "jet_pt_p"};
  map<string,string> pretty_cNames;
  pretty_cNames["num_jets_p"]="N_{j} #geq 1"; 
  pretty_cNames["jpsi_pt_p"]="p_{T}(J/#psi) > 20 GeV";    
  pretty_cNames["jpsi_eta_p"]="|#eta(J/#psi)| < 2.5";
  pretty_cNames["delta_r_p"]="#Delta R(Jet;J/#psi) < 0.4";    
  pretty_cNames["jet_eta_p"]="|#eta(jet)| < 2.5"; 
  pretty_cNames["jet_pt_p"]="p_{T}(jet) > 45 GeV";
  map<string,TH1D*> HistBook;
  HistBook["jet_pt"]=new TH1D("jet_pt","Jet p_{T};p_{T} [GeV];evts/binwidth",50,0,250);
  HistBook["jet_eta"]=new TH1D("jet_eta","Jet #eta;#eta;evts/binwidth",50,-2.6,2.6);
  HistBook["jet_e"]=new TH1D("jet_e","Jet E;E [GeV]; evts/binwidth",50,0,500);
  HistBook["jet_z"]=new TH1D("jet_z","Jet Z=p_{T}(J/#psi)/p_{T}(Jet);z;evts/binwidth",50,0,1.);
  HistBook["delta_r"]=new TH1D("delta_r","#Delta R(J/#psi,Jet); #Delta R; evts/binwidth",50,0,1.);
  HistBook["jpsi_pt"]=new TH1D("jpsi_pt","J/#psi p_{T};p_{T} [GeV];evts/binwidth",50,0,200);
  HistBook["jpsi_eta"]=new TH1D("jpsi_eta","J/#psi #eta;#eta;evts/binwidth",50,-2.6,2.6);
  HistBook["jpsi_e"]=new TH1D("jpsi_e","J/#psi E;E [GeV]; evts/binwidth",50,0,600);
  vector<string> plots = map_keys(HistBook);
  for(map<string,TH1D*>::iterator item=HistBook.begin(); item != HistBook.end(); ++item){
    item->second->Sumw2();
    item->second->SetLineWidth(0.5);
  }
  for(vector<string>::const_iterator p=plots.begin(); p!=plots.end(); ++p){
    const std::string& plot = *p;
    TCanvas canv(("canv_"+plot).c_str(),"Efficiency Ratio",1800,800);
    TPaveText label(0.08,0.6,0.18,0.71,"NB");
    TLatex cut_applied;
    cut_applied.SetTextAlign(12);
    cut_applied.SetTextSize(0.1);
    label.SetFillColor(0);
    label.SetBorderSize(0);
    label.AddText(HistBook[plot]->GetTitle());
    canv.Divide(3,2);
    canv.SetRightMargin(0);
    canv.SetTopMargin(0);
    for(size_t i = 0; i < sizeof(cut_branches)/sizeof(*cut_branches); i++){
      set_pad_margins(canv.cd(i+1),i+1);
      string hNRatio = plot +"_R_"+str_join("_",cut_branches,0,i+1);
      TH1D* ratio_hist = (TH1D*)HistBook[plot]->Clone(hNRatio.c_str());
      make_ratio_hist(ratio_hist,CutTree, cut_branches, i,plot);
      ratio_hist->Draw();
      if( i < 3){ //top row
	remove_axis(ratio_hist->GetXaxis());
      }
      cut_applied.DrawLatex(0.2,i==5 ? 0.3 : 0.1,pretty_cNames[cut_branches[i]].c_str());
    }
    canv.cd(0);
    label.Draw();
    canv.SaveAs((plot+".pdf").c_str());
  }
  
  return 0;
}
