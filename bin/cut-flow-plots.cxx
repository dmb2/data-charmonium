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
#include "histo-utils.hh"

using namespace std;

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
    print_cut_hist(CutTree, cut_branches, nCuts, plot, 
		   HistBook[plot], pretty_cNames,
		   "_ratio.png", make_ratio_hist);
    print_cut_hist(CutTree, cut_branches, nCuts, plot, 
		   HistBook[plot], pretty_cNames,
		   "_normal.png" , make_normal_hist);
    if(plot=="pileup"){ 
      continue;
    }
    print_hist(CutTree,plot,HistBook2D[plot+"_rsp"],
	       "_nominal_response.png", make_response_hist);
    print_cut_hist(CutTree, cut_branches, nCuts, plot, 
		   HistBook2D[plot+"_rsp"], pretty_cNames,
		   "_response.png", make_response_hist);
  }
  return 0;
}
