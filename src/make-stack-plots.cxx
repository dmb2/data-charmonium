#include <cmath>
#include <cstdlib>
#include <map>
#include <string>

#include "TH1D.h"
#include "TTree.h"

#include "histo-utils.hh"
#include "stack-utils.hh"
#include "histo-meta-data.hh"
#include "root-sugar.hh"


void print_stack_plots(const char* master_fname, const char* sample_names[],
		       const size_t n_samp){
  map<string,TTree*> sample_trees;
  sample_trees["master"]=retrieve<TTree>(master_fname,"mini");
  char fname[256];
  for(size_t i=0; i < n_samp; i++){
    snprintf(fname,256,"%s.mini.root",sample_names[i]);
    sample_trees[sample_names[i]]=retrieve<TTree>(fname,"mini");
  }

  const char* cut_branches[]={/*"mu_trigger_p",*/ "num_jets_p",
			      "jpsi_pt_p", "jpsi_eta_p",
			      "delta_r_p", "jet_eta_p",
			      "jet_pt_p"};
  size_t nCuts=sizeof(cut_branches)/sizeof(*cut_branches);
  map<string,string> pretty_cNames;
  init_cut_names(pretty_cNames);
  
  map<string,TH1D*> HistBook;
  init_hist_book(HistBook);
  for(map<string,TH1D*>::iterator item=HistBook.begin(); 
      item != HistBook.end(); ++item){
    TH1D* hist = item->second;
    setup_hist(hist);
    hist->SetFillStyle(1001);
  }
  vector<string> plots = map_keys(HistBook);
  for(vector<string>::const_iterator p=plots.begin(); p!=plots.end(); ++p){
    const std::string& plot = *p;
    print_stack(sample_trees,plot,HistBook[plot],"_stack.pdf");
    // print_cut_stack(sample_trees,cut_branches,nCuts,plot,
    // 		    HistBook[plot],pretty_cNames,
    // 		    "_normal.pdf");
  }
}
