#include "AtlasStyle.hh"
#include "TROOT.h"
#include "TStyle.h"
#include "color.hh"
#include "root-sugar.hh"

void usage(const char* name){
  MSG("Usage: "<< name << " syst-up.mini.root syst-down.mini.root ");
  MSG("")
}

int main(const int argc, const char* argv[]){
  if(argc < 3){
    usage(argv[0]);
    return 0;
  }

  AtlasStyle style;
  style.SetAtlasStyle();
  gStyle->SetFrameLineWidth(0.0);
  double stops[]={0.0,0.25,0.5,0.75,1.0};
  heat_gradient(gStyle,stops,sizeof(stops)/sizeof(*stops));
  
  return 0;
}
