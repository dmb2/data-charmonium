#include <iostream>
#include <cmath>
#include <cstdlib>
#include <map>
#include <vector>
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
  MSG("Usage: "<< name << " -i input.mini.root ");
}

int main(const int argc, char* const argv[]){
  char* in_fname=nullptr;
  int c;
  while((c = getopt(argc,argv,"i:"))!= -1){
    switch(c){
    case 'i':
      in_fname = optarg;
      break;
    default:
      abort();
    }
  }
  if(in_fname==nullptr){
    usage(argv[0]);
    exit(1);
  }
  setup_global_style();
  TFile* file = TFile::Open(in_fname);
  TTree* tree = nullptr;
  file->GetObject("mini",tree);
  std::map<std::string,TH1D*> HistBook;
  init_hist_book(HistBook);
  const char* plot_names[] = { "jet_pt","delta_r","jet_z",
			       "tau1","tau2","tau3",
			       "tau32","tau21"
  };
  std::vector<std::string> plots(plot_names,plot_names + sizeof(plot_names)/sizeof(*plot_names));
  for(std::vector<std::string>::const_iterator p=plots.begin(); p!=plots.end(); ++p){
    const std::string& plot = *p;
    MSG_DEBUG(plot);
  } 
  file->Close();
  return 0;
}
