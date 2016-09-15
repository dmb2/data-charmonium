#include "root-sugar.hh"

void usage(const char* prog_name){
  MSG("Usage: "<<prog_name<< " -i inFile -o outfile");
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
  TFile* file = new TFile(out_fname,"RECREATE");
  
  file->Close();
  if(file) delete file;
  return 0;
}
