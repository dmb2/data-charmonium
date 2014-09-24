#include <iostream>
#include <map>
#include <string>

#include "TFile.h"
#include "TTree.h"
#include "TCanvas.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TStyle.h"
#include "TLatex.h"
#include "TColor.h"
#include "TROOT.h"

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
void draw_histo(TTree* tree,const char* branch_name, const char* hist_name, 
		const char* cut_expr){
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
TH1* make_response_hist(TH1* base_hist, TTree* tree, 
			const char* cut_branches[],size_t cut_index, 
			const string& plot){
  TH1* response = (TH1*)base_hist->Clone((plot + "_RSP_"+
					  str_join("_",cut_branches,
						   0,cut_index+1)).c_str());
  draw_histo(tree,(plot+":truth_"+plot).c_str(),response->GetName(),
  	     str_join("*",cut_branches,0,cut_index+1).c_str());
  return response;
}
TH1* make_normal_hist(TH1* base_hist, TTree* tree, 
		      const char* cut_branches[], size_t cut_index, 
		      const string& plot){
  TH1* hist = (TH1*)base_hist->Clone((plot + "_NRM_"+
				      str_join("_",cut_branches,
					       0,cut_index+1)).c_str());
  draw_histo(tree,plot.c_str(),hist->GetName(),
	     str_join("*",cut_branches,0,cut_index+1).c_str());
  return hist;
}
TH1* make_ratio_hist(TH1* base_hist, TTree* tree,
		     const char* cut_branches[],size_t cut_index, 
		     const string& plot){
  TH1* ratio =(TH1*)base_hist->Clone((plot +"_R_"+
				      str_join("_",cut_branches,
					       0,cut_index+1)).c_str());
  string hNDenom = plot+str_join("_",cut_branches,0,cut_index)+"_tmp";
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
TH1* make_response_hist(TH1* base_hist,TTree* tree,const std::string& plot){
  TH1* hist = (TH1*)base_hist->Clone((plot + "_rsp_NOM").c_str());
  draw_histo(tree,(plot + ":truth_"+plot).c_str(), hist->GetName(), "");
  return hist;
}
TH1* make_normal_hist(TH1* base_hist,TTree* tree,const std::string& plot){
  TH1* hist = (TH1*)base_hist->Clone((plot + "_NOM").c_str());
  draw_histo(tree,plot.c_str(), hist->GetName(), "");
  return hist;
}

void print_hist(TTree* tree, const std::string& plot, 
		TH1* base_hist, const std::string suffix, 
		TH1* (*make_hist)(TH1*,TTree*,const std::string&)){
  TCanvas canv(("canv_"+plot).c_str(),"Plot",600,600);
  TLatex decorator;
  decorator.SetTextSize(0.04);
  TH1* hist = make_hist(base_hist,tree,plot);
  hist->Draw("H");
  decorator.DrawLatexNDC(0.,0.05,hist->GetTitle());
  canv.SaveAs((plot+suffix).c_str());
}
void print_cut_hist(TTree* tree,const char* cut_branches[],size_t nCuts, 
		const std::string& plot, TH1* base_hist, 
		map<string,string>& CutNames, std::string file_suffix,
		TH1* (*make_hist)(TH1* ,TTree* , const char**, 
				  size_t, const std::string&)){
  TCanvas canv(("canv_"+plot).c_str(),"Cut Plot",1800,800);
  // TPaveText label(0.08,0.6,0.18,0.71,"NB");
  TLatex decorator;
  // decorator.SetTextAlign(12);
  decorator.SetTextSize(0.1);
  canv.Divide(3,2);
  canv.SetRightMargin(0);
  canv.SetTopMargin(0);
  for(size_t i = 0; i < nCuts; i++){
    set_pad_margins(canv.cd(i+1),i+1);
    TH1* hist = make_hist(base_hist,tree, cut_branches, i,plot);
    hist->Draw("BOX");
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
int main(const int argc, const char* argv[]){
  AtlasStyle style;
  style.SetAtlasStyle();
  gStyle->SetFrameLineWidth(0.0);
  // gStyle->SetPadTickX(0);
  // gStyle->SetPadTickY(0);
  vector<string> args;
  for(int i=0; i < argc; i++){
    args.push_back(string(argv[i]));
  }
  TFile InFile("cut_tree.root");
  TTree* CutTree = dynamic_cast<TTree*>(InFile.Get("mini"));
  const char* cut_branches[]={"num_jets_p", "mu_trigger_p", "jpsi_pt_p",    
			      /*"jpsi_eta_p",*/ "delta_r_p",    
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
  map<string,TH1D*> HistBook;
  HistBook["pileup"]=new TH1D("pileup","Average Interactions per Xing; #mu; evts/binwidth",41,-0.5,40.5);
  HistBook["jet_pt"]=new TH1D("jet_pt","Jet p_{T};p_{T} [GeV];evts/binwidth",50,0,250);
  HistBook["jet_eta"]=new TH1D("jet_eta","Jet #eta;#eta;evts/binwidth",50,-2.6,2.6);
  HistBook["jet_e"]=new TH1D("jet_e","Jet E;E [GeV]; evts/binwidth",50,0,500);
  HistBook["jet_z"]=new TH1D("jet_z","Jet Z;z;evts/binwidth",50,0,1.);
  HistBook["delta_r"]=new TH1D("delta_r","#Delta R(J/#psi,Jet); #Delta R; evts/binwidth",50,0,1.);
  HistBook["jpsi_pt"]=new TH1D("jpsi_pt","J/#psi p_{T};p_{T} [GeV];evts/binwidth",50,0,200);
  HistBook["jpsi_eta"]=new TH1D("jpsi_eta","J/#psi #eta;#eta;evts/binwidth",50,-2.6,2.6);
  HistBook["jpsi_e"]=new TH1D("jpsi_e","J/#psi E;E [GeV]; evts/binwidth",50,0,600);
  vector<string> plots = map_keys(HistBook);
  map<string,TH2D*> HistBook2D;
  TColor* color = new TColor(1756,0.0,0.0,0.0,"tran_black",0.75);
  for(map<string,TH1D*>::iterator item=HistBook.begin(); 
      item != HistBook.end(); ++item){
    const std::string& plot = item->first;
    TH1D* hist = item->second;
    const TAxis* axis = hist->GetXaxis();
    hist->Sumw2();
    hist->SetMarkerStyle(1);
    hist->SetLineWidth(1.);
    hist->SetDrawOption("H");
    TH2D* hist2D = new TH2D((plot + "_rsp").c_str(), hist->GetTitle(),
			    axis->GetNbins(), axis->GetXmin(), axis->GetXmax(),
			    axis->GetNbins(), axis->GetXmin(), axis->GetXmax());
    hist2D->SetMarkerStyle(6);
    //hist2D->SetMarkerSize();
    hist2D->SetMarkerColor(color->GetNumber());

    hist2D->GetXaxis()->SetTitle("Truth");
    hist2D->GetYaxis()->SetTitle("Reconstructed");
    HistBook2D[plot+"_rsp"]=hist2D;
  }
  for(vector<string>::const_iterator p=plots.begin(); p!=plots.end(); ++p){
    const std::string& plot = *p;
    print_hist(CutTree,plot,HistBook[plot],
	       "_nominal.png", make_normal_hist);
    print_hist(CutTree,plot,HistBook2D[plot+"_rsp"],
	       "_nominal_response.png", make_response_hist);
    print_cut_hist(CutTree, cut_branches, nCuts, plot, 
		   HistBook[plot], pretty_cNames,
		   "_ratio.png", make_ratio_hist);
    print_cut_hist(CutTree, cut_branches, nCuts, plot, 
		   HistBook2D[plot+"_rsp"], pretty_cNames,
		   "_response.png", make_response_hist);
    print_cut_hist(CutTree, cut_branches, nCuts, plot, 
		   HistBook[plot], pretty_cNames,
		   "_normal.png" , make_normal_hist);
  }
  return 0;
}
