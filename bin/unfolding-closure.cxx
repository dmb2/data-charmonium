#include <iostream>
#include <map>
#include <string>
#include "TH1D.h"
#include "TTree.h"

#include "histo-meta-data.hh"
#include "histo-utils.hh"
#include "root-sugar.hh"

void usage(const char* name){
  MSG("Usage: "<< name << "-l lumi -i input.root -o output.root");
}


int main(const int argc, char* const argv[]){
  char* in_fname=nullptr;
  char* out_fname=nullptr;
  double lumi;
  int c;
  while((c = getopt(argc,argv,"l:i:o:"))!= -1){
    switch(c){
    case 'i':
      in_fname=optarg;
      break;
    case 'o':
      out_fname=optarg;
      break;
    case 'l':
      lumi = atof(optarg);
      break;
    default:
      abort();
    }
  }
  if(in_fname==nullptr ||
     out_fname==nullptr ||
     !std::isfinite(lumi)){
    usage(argv[0]);
    exit(1);
  }
  TTree* tree = retrieve<TTree>(in_fname,"mini");
  
  std::map<std::string,TH1D*> HistBook;
  init_hist_book(HistBook);
  char weight_expr[20];
  snprintf(weight_expr,20,"weight*%.4g*SF",lumi);
  TFile out_file(out_fname,"RECREATE");
  for(std::map<std::string,TH1D*>::iterator it=HistBook.begin();
      it!=HistBook.end(); ++it){
    const std::string& name = it->first;
    TH1* hist = make_normal_hist(it->second,tree,name.c_str(),"weight");
    hist->SetName(name.c_str());
  }
  out_file.Write();
  out_file.Close();
    
  return 0;
}
