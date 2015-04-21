#include <iostream>
#include <map>
#include <string>

#include "TFile.h"
#include "TTree.h"
#include "TCanvas.h"
#include "TColor.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TStyle.h"
#include "TLatex.h"
#include "TROOT.h"

#include "AtlasStyle.hh"
#include "root-sugar.hh"
#include "histo-utils.hh"
#include "histo-meta-data.hh"
#include "color.hh"

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
  double stops[]={0.0,0.25,0.5,0.75,1.0};
  heat_gradient(gStyle,stops,sizeof(stops)/sizeof(*stops));
  TTree* CutTree = retrieve<TTree>(argv[1],"mini");

  const char* cut_branches[]={"num_jets_p", /*"mu_trigger_p",*/
			      "jpsi_pt_p", /*"jpsi_eta_p",*/
			      "delta_r_p", "jet_eta_p",
			      "jet_pt_p"};
  size_t nCuts=sizeof(cut_branches)/sizeof(*cut_branches);
  map<string,string> pretty_cNames;
  init_cut_names(pretty_cNames);
  map<string,TH1D*> HistBook;
  init_hist_book(HistBook);
  map<string,TH2D*> HistBook2D;
  for(map<string,TH1D*>::iterator item=HistBook.begin(); 
      item != HistBook.end(); ++item){
    const std::string& plot = item->first;
    TH1D* hist = item->second;
    setup_hist(hist);
    HistBook2D[plot+"_rsp"]=setup_response_hist(hist);
    HistBook2D[plot+"_rel_rsp"]=setup_rel_res_hist(hist);
    HistBook2D[plot+"_res_dif"]=setup_res_dif_hist(hist);
  }
  const char* plot_names[] = {"jet_pt","jet_eta","jet_e","jet_z"};
  vector<string> plots(plot_names,plot_names + sizeof(plot_names)/sizeof(*plot_names));// = map_keys(HistBook);
  
  const char* skip_plots[]={"pileup","jpsi_lxy", "psi_m","jpsi_s","jet_emfrac"};
  for(vector<string>::const_iterator p=plots.begin(); p!=plots.end(); ++p){
    const std::string& plot = *p;
    /*
    print_hist(CutTree,plot,HistBook[plot],
	       cut_branches, nCuts,
	       "_nominal.png", make_normal_hist);
    print_cut_hist(CutTree, cut_branches, nCuts, plot, 
		   HistBook[plot], pretty_cNames,
		   "_ratio.png", make_ratio_hist);
    print_cut_hist(CutTree, cut_branches, nCuts, plot, 
		   HistBook[plot], pretty_cNames,
		   "_normal.png" , make_normal_hist);
    */
    bool skip=false;
    for(size_t i=0; i < sizeof(skip_plots)/sizeof(*skip_plots); i++){
      if(plot == std::string(skip_plots[i])){
	skip=true;
      }
    }
    if(skip) continue;
    print_hist(CutTree,plot,HistBook2D[plot+"_rsp"],
	       cut_branches, nCuts,
	       "_nominal_response.root", make_response_hist);
    // print_cut_hist(CutTree, cut_branches, nCuts, plot, 
    // 		   HistBook2D[plot+"_rsp"], pretty_cNames,
    // 		   "_response.root", make_response_hist);
    // print_hist(CutTree,plot,HistBook2D[plot+"_rel_rsp"],
    // 	       cut_branches, nCuts,
    // 	       "_nominal_rel_res.root", make_rel_res_hist);
    print_hist(CutTree,plot,HistBook2D[plot+"_res_dif"],
    	       cut_branches, nCuts,
    	       "_nominal_res_dif.root", make_res_dif_hist);
  }
  return 0;
}
