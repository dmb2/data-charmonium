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
  const char* plot_names[] = {"jpsi_pt"/*,"jet_pt","jpsi_m","jpsi_tau"*/};
  std::vector<std::string> plots(plot_names,plot_names + LEN(plot_names));  
  // std::vector<std::string> plots = map_keys(HistBook);
  const char* cbs[]={"mu_trigger_p", "jpsi_pt_p",
		     "jpsi_rap_p", "mumu_eta_p",/*"jpsi_eta_p",*/
		     "num_jets_p","delta_r_p", "jet_eta_p","jet_pt_p"};
  std::vector<std::string> cut_branches(cbs,cbs + LEN(cbs));
  std::map<std::string,std::string> pretty_cNames;
  init_cut_names(pretty_cNames);

  for(std::vector<std::string>::const_iterator p=plots.begin(); p!=plots.end(); ++p){
    const std::string& plot = *p;
    // print_stack(sample_trees,plot,HistBook[plot],"_nom_stk.pdf", target_lumi);
    print_stack(sample_trees,plot,HistBook[plot],"_cut_stk.pdf", target_lumi,
		cut_branches);
    print_cut_stack(sample_trees, cut_branches, plot, HistBook[plot],
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
  setup_global_style();

  gStyle->SetFrameLineWidth(0.0);
  gStyle->SetPalette(1);
  print_stack_plots(argv[2],&argv[3],argc-3,atof(argv[1]));
  return 0;
}
