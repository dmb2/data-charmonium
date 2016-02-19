#include <iostream>
#include "root-sugar.hh"

void usage(const char* name){
  MSG("Usage: "<< name << " input.root tree_name");
}


int main(const int argc, const char* argv[]){
  const double pt_bins[] = {40.,60.,110.};
  double rap_bins[8];
  for(size_t i=0; i < LEN(rap_bins); i++ ){
    rap_bins[i]=i*0.25;
  }
  //double loop over bins, build cut_string and filename using
  //input.root as a basename.
  
  return 0;
}
