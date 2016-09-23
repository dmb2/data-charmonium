
#include "TLegend.h"
#include "TTree.h"
#include "TKey.h"
#include "TCanvas.h"
#include "TH1.h"
#include "TH2D.h"
#include "root-sugar.hh"
#include "TStyle.h"
#include "color.hh"
#include "histo-utils.hh"

#include "RooUnfoldResponse.h"
#include "RooUnfoldBinByBin.h"
#include "RooUnfoldBayes.h"
#include "RooUnfoldSvd.h"

void usage(const char* prog_name){
  MSG("Usage: "<<prog_name<< "-t truth_file.root -i in_file.root -o out_name");
}

int main(const int argc, char* const argv[]){
  char* in_fname=NULL;
  char* truth_fname=NULL;
  char* out_fname=NULL;
  int num_iter=4;
  bool print_validation_plots=false;
  int c;
  while((c = getopt(argc,argv,"vn:t:i:o:")) != -1){
    switch(c){
    case 'v':
      print_validation_plots=true;
      break;
    case 'n':
      num_iter=atoi(optarg);
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

  setup_global_style();
  // double stops[]={0.0,0.25,0.5,0.75,1.0};
  // heat_gradient(gStyle,stops,LEN(stops));
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
      double N_sig = hist->Integral();
      double N_MC = reco_hist->Integral();
      reco_hist->Scale(N_sig/N_MC);
      truth_hist->Scale(N_sig/N_MC);//n_truth?
      if(print_validation_plots){
	//TODO Polish these and put them in supplement
	TCanvas debug("debug","debug",600,600);
	TLegend* leg = init_legend();
	leg->AddEntry(reco_hist,"MC Reco","l");
	leg->AddEntry(truth_hist,"MC Truth","l");
	leg->AddEntry(hist,"Reco","l");
	reco_hist->SetLineColor(kBlue);
	reco_hist->Draw("HIST");
	truth_hist->SetLineColor(kRed);
	truth_hist->Draw("HIST same");
	hist->SetLineColor(kBlack);
	hist->Draw("HIST same");
	leg->Draw();
	debug.SaveAs((name+"reco_truth_debug.pdf").c_str());
	debug.Clear();
	response_hist->Draw("COLZ");
	debug.SaveAs((name+"response_debug.pdf").c_str());
      }
      TH1D* unfolded=NULL;
      RooUnfoldResponse response(reco_hist, truth_hist,response_hist,
				 (name+"_unfolded").c_str(),hist->GetTitle());
      // RooUnfoldBinByBin UnfoldBBB(&response,hist);
      RooUnfoldBayes unfold(&response,hist,num_iter);
      // unfold.SetSmoothing(true);
      MSG_DEBUG(unfold.Chi2(truth_hist,RooUnfold::kCovariance));
      unfolded = dynamic_cast<TH1D*>(unfold.Hreco(RooUnfold::kCovariance));
      /*
      for(size_t i=1; i < num_iter; i++){

      }
      */ 
      TCanvas canv("canv","unfolded",600,600);
      // TH1D* unfolded_bbb = dynamic_cast<TH1D*>(UnfoldBBB.Hreco(RooUnfold::kCovariance));
      // unfolded_bbb->SetLineColor(kBlue);
      // unfolded_bbb->Draw("e0");
      unfolded->SetLineColor(kBlue);
      unfolded->SetMarkerSize(0);
      unfolded->SetMarkerStyle(kDot);
      unfolded->Draw("H same");

      hist->SetLineColor(kBlack);
      hist->SetMarkerSize(0);
      hist->SetMarkerStyle(kDot);
      hist->Draw("H same");

      truth_hist->SetLineColor(kRed);
      truth_hist->SetMarkerSize(0);
      truth_hist->SetMarkerStyle(kDot);
      truth_hist->Draw("HIST same");
      TLegend* leg = init_legend();
      // leg->AddEntry(unfolded_bbb,"Unfolded Bin-by-Bin","l");
      leg->AddEntry(unfolded,"Iterative Bayes","l");
      leg->AddEntry(hist,"Reco","l");
      leg->AddEntry(truth_hist,"MC Truth","l");
      leg->Draw();
      canv.SaveAs((name+"_unfolded.pdf").c_str());
      // break;
    }
  }
  reco_file->Close();
  
  return 0;
}
