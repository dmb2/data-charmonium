#include <iostream>
#include <cmath>
#include <cstdlib>
#include <map>
#include <string>

#include "TROOT.h"
#include "TStyle.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TTree.h"

#include "histo-utils.hh"
#include "stack-utils.hh"
#include "histo-meta-data.hh"
#include "root-sugar.hh"

#include "AtlasStyle.hh"

void usage(const char* name){
  std::cout <<"Usage: "<< name << " target_lumi (fb) reference_file.root [mc_samples.root]"<<std::endl;
}
void print_stack_plots(const char* master_fname, const char* sample_names[],
		       const size_t n_samp, const double target_lumi){
  std::map<std::string,TTree*> sample_trees;
  sample_trees["master"]=retrieve<TTree>(master_fname,"mini");
  std::string fname;
  for(size_t i=0; i < n_samp; i++){
    fname=std::string(sample_names[i]);
    sample_trees[fname.substr(0,fname.find(".mini.root"))]=retrieve<TTree>(fname.c_str(),"mini");
  }
  std::map<std::string,TH1D*> HistBook;
  init_hist_book(HistBook);
  std::map<std::string,TH2D*> Hist2DBook;
  init_hist2D_book(Hist2DBook);
  for(std::map<std::string,TH1D*>::iterator item=HistBook.begin(); 
      item != HistBook.end(); ++item){
    setup_hist(item->second);
  }
  const char* plot_names[] = {"jet_pt", "jpsi_pt"};
  std::vector<std::string> plots(plot_names,plot_names + sizeof(plot_names)/sizeof(*plot_names));  
  // std::vector<std::string> plots = map_keys(HistBook);
  const char* cut_branches[]={"num_jets_p", /*"mu_trigger_p",*/
			      "jpsi_pt_p", /*"jpsi_eta_p",*/
			      "delta_r_p", "jet_eta_p",
			      "jet_pt_p"};
  size_t nCuts=sizeof(cut_branches)/sizeof(*cut_branches);
  std::map<std::string,std::string> pretty_cNames;
  init_cut_names(pretty_cNames);

  for(std::vector<std::string>::const_iterator p=plots.begin(); p!=plots.end(); ++p){
    const std::string& plot = *p;
    print_stack(sample_trees,plot,HistBook[plot],"_nom_stk.pdf", target_lumi);
    print_stack(sample_trees,plot,HistBook[plot],"_cut_stk.pdf", target_lumi,
		cut_branches,nCuts);
    print_cut_stack(sample_trees, cut_branches,nCuts, plot, HistBook[plot],
		    pretty_cNames,"_panel_stk.pdf",target_lumi);
  }
  // plots = map_keys(Hist2DBook);
  // for(std::vector<std::string>::const_iterator p=plots.begin(); p!=plots.end(); ++p){
  //   const std::string& plot = *p;
  //   print_2D_stack(sample_trees,plot,Hist2DBook[plot],"_stacked.pdf", target_lumi);
  //   print_2D_slices(sample_trees,plot,Hist2DBook[plot],"_sliced.pdf", target_lumi);
  // }    
}

int main(const int argc, const char* argv[]){
  if(argc < 3){
    usage(argv[0]);
    return 0;
  }
  AtlasStyle style;
  style.SetAtlasStyle();
  gStyle->SetFrameLineWidth(0.0);
  gStyle->SetPalette(1);
  print_stack_plots(argv[2],&argv[3],argc-3,atof(argv[1]));
  return 0;
}
