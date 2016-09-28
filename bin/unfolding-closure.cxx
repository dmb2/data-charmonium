#include <iostream>
#include <map>
#include <string>
#include "TH1D.h"

#include "histo-meta-data.hh"
#include "root-sugar.hh"

void usage(const char* name){
  MSG("Usage: "<< name << " -i input.root -o output.root");
}


int main(const int argc, char* const argv[]){
  char* in_fname=nullptr;
  char* out_fname=nullptr;
  int c;
  while((c = getopt(argc,argv,"i:o:"))!= -1){
    switch(c){
    case 'i':
      in_fname=optarg;
      break;
    case 'o':
      out_fname=optarg;
      break;
    default:
      abort();
    }
  }
  if(optind==argc || in_fname==nullptr ||
     out_fname==nullptr){
    usage(argv[0]);
    exit(1);
  }
  return 0;
}
