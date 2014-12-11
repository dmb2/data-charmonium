#include <iostream>
#include <cmath>
#include <cstdlib>
#include <map>
#include <string>

#include "TROOT.h"
#include "TStyle.h"
#include "TH1D.h"
#include "TTree.h"

#include "histo-utils.hh"
#include "stack-utils.hh"
#include "histo-meta-data.hh"
#include "root-sugar.hh"

#include "AtlasStyle.hh"

void usage(const char* name){
  std::cout <<"Usage: "<< name << " input_file.root"<<std::endl;
}
void print_stack_plots(const char* master_fname, const char* sample_names[],
		       const size_t n_samp){
  std::map<std::string,TTree*> sample_trees;
  sample_trees["master"]=retrieve<TTree>(master_fname,"mini");
  char fname[512];
  for(size_t i=0; i < n_samp; i++){
    snprintf(fname,256,"%s.mini.root",sample_names[i]);
    sample_trees[sample_names[i]]=retrieve<TTree>(fname,"mini");
  }

  const char* cut_branches[]={/*"mu_trigger_p",*/ "num_jets_p",
			      "jpsi_pt_p", "jpsi_eta_p",
			      "delta_r_p", "jet_eta_p",
			      "jet_pt_p"};
  size_t nCuts=sizeof(cut_branches)/sizeof(*cut_branches);
  std::map<std::string,std::string> pretty_cNames;
  init_cut_names(pretty_cNames);
  
  std::map<std::string,TH1D*> HistBook;
  init_hist_book(HistBook);
  for(std::map<std::string,TH1D*>::iterator item=HistBook.begin(); 
      item != HistBook.end(); ++item){
    TH1D* hist = item->second;
    setup_hist(hist);
    hist->SetFillStyle(1001);
  }
  std::vector<std::string> plots = map_keys(HistBook);
  for(std::vector<std::string>::const_iterator p=plots.begin(); p!=plots.end(); ++p){
    const std::string& plot = *p;
    print_stack(sample_trees,plot,HistBook[plot],"_stack.root");
    // print_cut_stack(sample_trees,cut_branches,nCuts,plot,
    // 		    HistBook[plot],pretty_cNames,
    // 		    "_normal.pdf");
  }
}

int main(const int argc, const char* argv[]){
  if(argc < 2){
    usage(argv[0]);
    return 0;
  }
  AtlasStyle style;
  style.SetAtlasStyle();
  gStyle->SetFrameLineWidth(0.0);
  //const char* sample_names[]={"1S0_8","3S1_8","3PJ_8","3S1_1","3PJ_1"};
  print_stack_plots(argv[1],&argv[2],argc-2);
  return 0;
}
