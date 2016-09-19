#include "TTree.h"
#include "TCanvas.h"
#include "TH1.h"
#include "root-sugar.hh"

void usage(const char* prog_name){
  MSG("Usage: "<<prog_name<< "-t truth_file.root -i in_file.root -o out_name");
}

int main(const int argc, char* const argv[]){
  char* in_fname=NULL;
  char* truth_fname=NULL;
  char* out_fname=NULL;
  int c;
  while((c = getopt(argc,argv,"t:i:o:")) != -1){
    switch(c){
    case 't':
      truth_fname = optarg;
      break;
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
  if(in_fname==NULL || truth_fname==NULL){
    usage(argv[0]);
    exit(1);
  }

  MSG("Input File: "<<in_fname);
  MSG("Truth File: "<<truth_fname);
  MSG("Output File: "<<out_fname);
  TTree* tree = retrieve<TTree>(truth_fname,"mini");
  TCanvas* canv = retrieve<TCanvas>(in_fname,"canv");
  //Find signal hist in the input file
  TObject* obj=NULL;
  TIter iter(canv->GetListOfPrimitives());
  while((obj=iter())){
    if(obj->InheritsFrom("TH1")){
      MSG_DEBUG(obj->GetName());
    }
  }
  return 0;
}
