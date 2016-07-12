#include "root-sugar.hh"
#include "getopt.h"

#include "TClass.h"
#include "TKey.h"
#include "TObject.h"
#include "TH1D.h"
#include "TMath.h"
#include "histo-utils.hh"

void usage(const char* name){
  MSG("Usage: "<< name << " -o output.root [input_files.root]");
  MSG("Adds up a list of root files containing systematic histograms to\n produce a file representing the total for that sample");
}
int main(const int argc, char* const argv[]){
  char* outFName=NULL;
  int c;
  while((c = getopt(argc,argv,"o:")) != -1){
    switch(c){
    case 'o':
      outFName=optarg;
      break;
    default:
      abort();
    }
  }
  int idx=optind;
  if(outFName==NULL || idx==argc){
    usage(argv[0]);
    exit(1);
  }
  TFile outFile(outFName,"RECREATE");
  std::vector<TFile*> files;
  for(int i=idx; i < argc; i++){
    MSG_DEBUG("Opening: "<<argv[i]);
    files.push_back(TFile::Open(argv[i]));
    MSG_DEBUG(files.back()->GetName());
  }
  TIter next(files[0]->GetListOfKeys());
  std::vector<TH1*> histos;
  TKey *key=NULL;
  // make local copies of all histograms in the first file
  while( (key = dynamic_cast<TKey*>(next()) ) ){
    std::string name(key->GetName());
    if(std::string(key->GetClassName())=="TH1D"){
      if(name.find("_nominal")==std::string::npos){
	TH1D* hist = dynamic_cast<TH1D*>(key->ReadObj()->Clone((name+"_total").c_str()));
	for(int i=0; i < hist->GetNbinsX(); i++){
	  double err=hist->GetBinError(i);
	  MSG_DEBUG("Bin: "<<i<<" error: "<<err);
	  hist->SetBinError(i,err*err);
	}
	histos.push_back(hist);
      }
    }
  }
  outFile.cd();
  for(std::vector<TH1*>::iterator h=histos.begin(); h!=histos.end(); ++h){
    std::string name((*h)->GetName());
    name=name.substr(0,name.size()-6);
    MSG_DEBUG("Processing: "<<(*h)->GetName());
    for(std::vector<TFile*>::iterator file = files.begin()+1; file!=files.end(); ++file){
      MSG_DEBUG("\t File: "<<(*file)->GetName());
      TH1D* hist = dynamic_cast<TH1D*>((*file)->Get(name.c_str()));
      for(int i=0; i < hist->GetNbinsX(); i++){
      	double err=hist->GetBinError(i);
      	double curr_err2=(*h)->GetBinError(i);
      	(*h)->SetBinError(i,curr_err2+err*err);
      }
    }
    for(int i=0; i < (*h)->GetNbinsX(); i++){
      double err2=(*h)->GetBinError(i);
      (*h)->SetBinError(i,TMath::Sqrt(err2));
    }
    (*h)->Write();
  }
  outFile.Close();
  return 0;
}
