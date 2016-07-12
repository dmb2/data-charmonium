#include <cmath>
#include <cstdlib>
#include <iostream>
#include <map>
#include <string>

#include "TH1D.h"
#include "TStyle.h"
#include "TTree.h"
#include "TROOT.h"

#include "histo-utils.hh"
#include "stack-utils.hh"
#include "histo-meta-data.hh"
#include "root-sugar.hh"

using namespace std;
void usage(const char* name){
  cout <<"Usage: "<< name << " input_file.root"<<endl;
}

int main(const int argc, const char* argv[]){
  if(argc!=2){
    usage(argv[0]);
    return 0;
  }
  setup_global_style();

  map<string,TTree*> sample_trees;
  sample_trees["master"]=retrieve<TTree>(argv[1],"mini");
  const char* sample_names[]={"1S0_8","3S1_8","3PJ_8","3S1_1","3PJ_1"};
  char fname[256];
  for(size_t i=0; i < sizeof(sample_names)/sizeof(*sample_names); i++){
    snprintf(fname,256,"%s.mini.root",sample_names[i]);
    sample_trees[sample_names[i]]=retrieve<TTree>(fname,"mini");
  }
  /*
  const char* cbs[]={"num_jets_p", "jpsi_pt_p",    
		     "jpsi_eta_p", "delta_r_p",    
		     "jet_eta_p", "jet_pt_p"};
  std::vector<std::string> cut_branches(cbs,cbs + sizeof(cbs)/sizeof(*cbs));
  */
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
    // print_cut_stack(sample_trees,cut_branches,plot,
    // 		    HistBook[plot],pretty_cNames,
    // 		    "_normal.pdf");
  }
  return 0;
}
