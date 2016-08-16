#include "root-sugar.hh"
#include "TCanvas.h"
#include "TROOT.h"
#include "TColor.h"
#include "TH1.h"
#include "TFile.h"
#include "TLatex.h"
#include "color.hh"
#include "histo-utils.hh"
#include "TStyle.h"

#include <string>
#include <math.hh>

void usage(const char* prog_name){
  MSG("Usage: "<<prog_name<< " -o outfile [slice files]");
}
int main(const int argc, char* const argv[]){
  char* out_fname=NULL;
  int c;
  while((c = getopt(argc,argv,"o:")) != -1){
    switch(c){
    case 'o':
      out_fname = optarg;
      break;
    default:
      abort();
    }
  }
  if(out_fname==NULL || optind==argc){
    usage(argv[0]);
    exit(1);
  }
  MSG_DEBUG("Writing to: "<< out_fname);
  TCanvas canv("canvas","canvas",1200,600*)
  for(int i=optind; i < argc; i++){
    const std::string fname(argv[i]);
    MSG_DEBUG("Processing "<<fname);
    std::string slice_dir;
    std::vector<std::string> parts = split_string(fname,'/');
    for(std::vector<std::string>::const_iterator str_itr = parts.begin();
	str_itr!=parts.end(); ++str_itr){
      if(str_itr->find("jpsi_pt")!=std::string::npos){
	slice_dir = *str_itr;
	break;
      }
    }
    parts = split_string(slice_dir,'_');
    int hi_edge = atoi(parts.at(parts.size()-2).c_str());
    int lo_edge = atoi(parts.at(parts.size()-1).c_str());
    
    // MSG_DEBUG("High edge: "<<hi_edge/100.0<<" Low edge: "<<lo_edge/100.0);
  }
  
  return 0;
}
