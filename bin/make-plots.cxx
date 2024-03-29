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
#include "plot-utils.hh"
#include "histo-meta-data.hh"
#include "color.hh"
#include "root-sugar.hh"

void usage(const char* name){
  MSG("Usage: "<< name << " -l target_lumi (fb) [mc_samples.root]");
}
void print_plots(const size_t idx, char* const sample_names[], 
		 const size_t n_samp, const double target_lumi){
  std::map<std::string,TTree*> sample_trees;
  for(size_t i=idx; i < n_samp; i++){
    // strip .mini.root off
    std::string name = std::string(sample_names[i]);
    size_t pos = name.find(".mini.root");
    sample_trees[name.substr(0,pos)]=retrieve<TTree>(name.c_str(),"mini");
  }

  std::map<std::string,TH1D*> HistBook;
  init_hist_book(HistBook);
  std::map<std::string,TH2D*> Hist2DBook;
  init_hist2D_book(Hist2DBook);
  for(std::map<std::string,TH1D*>::iterator item=HistBook.begin();
      item != HistBook.end(); ++item){
    TH1D* hist = item->second;
    setup_hist(hist);
  }
  std::vector<std::string> plots = map_keys(HistBook);
  for(std::vector<std::string>::const_iterator p=plots.begin(); p!=plots.end(); ++p){
    const std::string& plot = *p;
    print_ratio_hist(sample_trees,plot,HistBook[plot],".pdf", target_lumi);
  }
  /*
  plots = map_keys(Hist2DBook);
  for(std::vector<std::string>::const_iterator p=plots.begin(); p!=plots.end(); ++p){
    const std::string& plot = *p;
    print_2D_stack(sample_trees,plot,Hist2DBook[plot],".pdf", target_lumi);
    print_2D_slices(sample_trees,plot,Hist2DBook[plot],"_sliced.pdf", target_lumi);
  }
  */
}

int main(const int argc, char* const argv[]){
  double lumi;
  int c;
  while((c = getopt(argc,argv,"l:"))!= -1){
    switch(c){
    case 'l':
      lumi=atof(optarg);
      break;
    default:
      abort();
    }
  }
  if(optind==argc || !std::isfinite(lumi)){
    usage(argv[0]);
    exit(1);
  }
  setup_global_style();
  double stops[]={0.0,0.25,0.5,0.75,1.0};
  heat_gradient(gStyle,stops,LEN(stops));
  print_plots(optind,argv,argc,lumi);
  return 0;
}
