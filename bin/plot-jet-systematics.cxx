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
  
  return 0;
}
