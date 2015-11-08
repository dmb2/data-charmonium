#include "AtlasStyle.hh"
#include "TTree.h"
#include "TROOT.h"
#include "TStyle.h"
#include "color.hh"
#include "root-sugar.hh"
#include "histo-meta-data.hh"
#include "histo-utils.hh"
#include "TH1D.h"
#include "TH1.h"
#include "plot-utils.hh"

void usage(const char* name){
  MSG("Usage: "<< name << " syst-up.mini.root syst-down.mini.root nominal.root output.root");
  MSG("Creates histograms representing fractional systematic error of inputs");
  
}

int main(const int argc, const char* argv[]){
  if(argc < 5){
    usage(argv[0]);
    return 0;
  }
  const char* syst_up_fname=argv[1];
  const char* syst_down_fname=argv[2];
  const char* nominal_fname = argv[3];
  const char* outFName = argv[4];
  MSG_DEBUG("Using up file:"<<syst_up_fname);
  MSG_DEBUG("Using down file: "<<syst_down_fname);
  MSG_DEBUG("Using nominal file: "<<nominal_fname);
  MSG_DEBUG("Writing output to: "<<outFName);
  TTree* syst_up_tree = retrieve<TTree>(syst_up_fname,"mini");
  TTree* syst_down_tree = retrieve<TTree>(syst_down_fname,"mini");
  TTree* nominal_tree = retrieve<TTree>(nominal_fname,"mini");

  AtlasStyle style;
  style.SetAtlasStyle();
  gStyle->SetFrameLineWidth(0.0);
  double stops[]={0.0,0.25,0.5,0.75,1.0};
  heat_gradient(gStyle,stops,sizeof(stops)/sizeof(*stops));
  // loop over all histograms
  // compute (syst-up - syst-down)/nominal
  // save result to file
  hist_2D_book Hist2DBook;
  hist_book HistBook;
  init_hist2D_book(Hist2DBook);
  init_hist_book(HistBook);
  // std::vector<std::string> plots = map_keys(HistBook);
  TFile output(outFName,"RECREATE");
  // output.cd();
  for(hist_book::iterator entry=HistBook.begin(); entry!=HistBook.end(); ++entry){
    const std::string& plot = entry->first;
    TH1D* base_hist = entry->second;
    MSG_DEBUG("Processing: "<<plot);
    TH1* syst_up_hist = make_normal_hist(base_hist, syst_up_tree,plot,"weight","_syst_up");
    TH1* syst_down_hist = make_normal_hist(base_hist,syst_down_tree,plot,"weight","_syst_down");
    TH1* nominal_hist = make_normal_hist(base_hist,nominal_tree,plot,"weight","_nominal");
    TH1D* result = dynamic_cast<TH1D*>(base_hist->Clone((plot + "_syst").c_str()));
    result->Add(syst_up_hist,syst_down_hist,1,-1);
    result->Divide(nominal_hist);
    result->Write();
  }
  // output.Write();
  output.Close();
  return 0;
}
