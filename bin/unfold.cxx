#include "TTree.h"
#include "TKey.h"
#include "TCanvas.h"
#include "TH1.h"
#include "TH2D.h"
#include "root-sugar.hh"
#include "histo-utils.hh"

void usage(const char* prog_name){
  MSG("Usage: "<<prog_name<< "-t truth_file.root -i in_file.root -o out_name");
}

int main(const int argc, char* const argv[]){
  char* in_fname=NULL;
  char* truth_fname=NULL;
  char* out_fname=NULL;
  bool print_validation_plots=false;
  int c;
  while((c = getopt(argc,argv,"vt:i:o:")) != -1){
    switch(c){
    case 'v':
      print_validation_plots=true;
      break;
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
  TFile* reco_file = TFile::Open(in_fname);
  TIter iter(reco_file->GetListOfKeys());
  TKey *key=NULL;
  while((key=(TKey*)iter())){
    if(std::string(key->GetClassName()).find("TH")!=std::string::npos){
      TH1* hist = dynamic_cast<TH1*>(key->ReadObj());
      if(hist==nullptr){
	MSG_ERR("Error casting hist of type: "<<key->ClassName()<<" to TH1*, object named "<<key->GetName());
      }
      const std::string name(hist->GetName());
      TH1* base_hist = dynamic_cast<TH1*>(hist->Clone(name.c_str()));
      for(int i =0; i < base_hist->GetNbinsX(); i++){
	base_hist->SetBinContent(i,0);
	base_hist->SetBinError(i,0);
      }
      TH2D* response_hist = setup_response_hist(base_hist);
      response_hist = dynamic_cast<TH2D*>(make_response_hist(response_hist,tree,base_hist->GetName(),"SF*weight","_response"));
      TH1* reco_hist = make_normal_hist(base_hist,tree,base_hist->GetName(),"SF*weight","_reco");
      TH1* truth_hist = make_normal_hist(base_hist,tree,(std::string("truth_")+base_hist->GetName()),"SF*weight","_truth");
      if(print_validation_plots){
	//TODO Polish these and put them in supplement
	TCanvas debug("debug","debug",600,600);
	reco_hist->SetLineColor(kBlue);
	reco_hist->Draw("HIST");
	truth_hist->SetLineColor(kRed);
	truth_hist->Draw("HIST same");
	debug.SaveAs((name+"reco_truth_debug.pdf").c_str());
	debug.Clear();
	response_hist->Draw("COLZ");
	debug.SaveAs((name+"response_debug.pdf").c_str());
      }
    }
  }
  reco_file->Close();
  
  return 0;
}
