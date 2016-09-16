#include "TTree.h"
#include "root-sugar.hh"

void usage(const char* prog_name){
  MSG("Usage: "<<prog_name<< "-t truth_file.root -i in_file.root -o out_name");
}

int main(const int argc, char* const argv[]){
  char* in_fname=NULL;
  char* out_fname=NULL;
  int c;
  while((c = getopt(argc,argv,"i:o:")) != -1){
    switch(c){
    case 'i':
      in_fname = optarg;
      break;
    case 'o':
      out_fname = optarg;
      break;
    default:
      abort();
    }
  }
  if(in_fname==NULL){
    usage(argv[0]);
    exit(1);
  }

  MSG("Input File: "<<in_fname);
  MSG("Output File: "<<out_fname);
  // TFile* file = new TFile(out_fname,"RECREATE");
  TTree* tree = retrieve<TTree>(in_fname,"mini");
  const char* vars[] = {
    // "jet_pt","jet_eta","jet_phi","jet_e","jet_m",
    // "jpsi_pt","jpsi_eta","jpsi_phi","jpsi_e","jpsi_m",
    "tau1","tau2","tau3","tau21","tau32",
    "jet_z","delta_r"
  };
  for(size_t i=0; i < LEN(vars); i++){
  // Reco
  
  // Truth
  
  // Reco:Truth
  }
  
  // file->Close();
  // if(file) delete file;
  return 0;
}
