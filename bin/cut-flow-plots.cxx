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
#include "TROOT.h"

#include "AtlasStyle.hh"
#include "root-sugar.hh"
#include "histo-utils.hh"
#include "histo-meta-data.hh"

using namespace std;
void usage(const char* name){
  cout <<"Usage: "<< name << " input_file.root"<<endl;
}

int main(const int argc, const char* argv[]){
  if(argc!=2){
    usage(argv[0]);
    return 0;
  }
  AtlasStyle style;
  style.SetAtlasStyle();
  gStyle->SetFrameLineWidth(0.0);
  TTree* CutTree = retrieve<TTree>(argv[1],"mini");

  const char* cut_branches[]={"num_jets_p", "mu_trigger_p", "jpsi_pt_p",    
			      /*"jpsi_eta_p",*/ "delta_r_p",    
			      "jet_eta_p", "jet_pt_p"};
  size_t nCuts=sizeof(cut_branches)/sizeof(*cut_branches);
  map<string,string> pretty_cNames;
  initialize_cut_names(pretty_cNames);
  map<string,TH1D*> HistBook;
  initialize_hist_book(HistBook);
  map<string,TH2D*> HistBook2D;
  for(map<string,TH1D*>::iterator item=HistBook.begin(); 
      item != HistBook.end(); ++item){
    const std::string& plot = item->first;
    TH1D* hist = item->second;
    setup_hist(hist);
    HistBook2D[plot+"_rsp"]=setup_response_hist(hist);
  }

  vector<string> plots = map_keys(HistBook);
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
