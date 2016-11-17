
#include "TLegend.h"
#include "TLatex.h"
#include "TTree.h"
#include "TKey.h"
#include "TCanvas.h"
#include "TH1.h"
#include "TH2D.h"
#include "root-sugar.hh"
#include "TStyle.h"
#include "color.hh"
#include "histo-utils.hh"
#include "unfolding-utils.hh"

#include "RooUnfoldResponse.h"
#include "RooUnfoldBinByBin.h"
#include "RooUnfoldBayes.h"
#include "RooUnfoldSvd.h"

void usage(const char* prog_name){
  MSG("Usage: "<<prog_name<< " -t truth_file.root -i in_file.root ");
}

int main(const int argc, char* const argv[]){
  char* in_fname=NULL;
  char* truth_fname=NULL;
  bool do_syst=false;
  int num_iter(0);
  int c;
  while((c = getopt(argc,argv,"st:i:")) != -1){
    switch(c){
    case 't':
      truth_fname = optarg;
      break;
    case 'i':
      in_fname = optarg;
      break;
    case 's':
      do_syst = true;
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
  MSG("Input File: "<<in_fname);
  MSG("Truth File: "<<truth_fname);
  TTree* tree = retrieve<TTree>(truth_fname,"mini");
  TFile* reco_file = TFile::Open(in_fname);
  
  const char* variables[] = {"delta_r","jet_pt","jet_z",
			     // "jpsi_pt","jpsi_eta"//,"tau1","tau2","tau3","tau32","tau21"
  };
  int n_evts = tree->GetEntries();
  for(size_t i=0; i < LEN(variables); i++){
    TH1D* reco_hist=nullptr;
    reco_file->GetObject(variables[i],reco_hist);
    if(!reco_hist){
      MSG_ERR("Could not retrieve object named: "<<variables[i]);
      exit(1);
    }
    std::string name(reco_hist->GetName());
    // reco_hist->SetName((name+"_reco").c_str());
    TH1* base_hist = dynamic_cast<TH1D*>(reco_hist->Clone(name.c_str()));
    TH2* response_hist = mc_response(base_hist,tree,n_evts);
    num_iter = get_iterations(base_hist,response_hist,int(reco_hist->Integral()));
    TH1* unfolded = unfold(response_hist,reco_hist,num_iter,name);
    unfolded->GetXaxis()->SetTitle(base_hist->GetXaxis()->GetTitle());
    TCanvas canv("canv","canv",600,600);
    unfolded->SetMarkerStyle(kFullDotLarge);
    unfolded->SetMarkerSize(0.75);
    int width=6;
    unfolded->SetLineWidth(width);
    unfolded->DrawCopy("e1 x0");
    if(do_syst){
      //square peg, round hole. I'm pidgeon-holing num_iter into a string... what could go wrong
      char iter_str[20];
      snprintf(iter_str,20,"%d",num_iter);
      TH1* syst_err_hist = build_syst_err_hist(reco_hist,"total",iter_str,unfold_syst_err);
      TH1* splot_err_hist = nullptr;
      reco_file->GetObject((std::string(variables[i])+"_sig_tot_err").c_str(),splot_err_hist);
      if(splot_err_hist!=nullptr){
	MSG_DEBUG("Adding splot syst hist");
	width-=2;
	unfolded->SetLineWidth(width);
	unfolded->Add(splot_err_hist);
	unfolded->DrawCopy("e1 x0 same");
	// syst_err_hist->Add(splot_err_hist);
      }
      TH1* unfold_err_hist = build_unfold_err_hist(unfolded,response_hist,num_iter,name);
      // dump_hist(unfold_err_hist);
      unfolded->Add(unfold_err_hist);
      width-=2;
      unfolded->Add(syst_err_hist);
      unfolded->SetLineWidth(width);
    }
    unfolded->DrawCopy("e1 x0 same");
    canv.SaveAs((std::string(variables[i])+"_unfolded.pdf").c_str());
  }
  reco_file->Close();
  
  return 0;
}
