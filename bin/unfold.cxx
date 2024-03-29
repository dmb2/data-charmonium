#include "TColor.h"
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
#include "histo-meta-data.hh"
#include "unfolding-utils.hh"

#include "RooUnfoldResponse.h"
#include "RooUnfoldBinByBin.h"
#include "RooUnfoldBayes.h"
#include "RooUnfoldSvd.h"

void usage(const char* prog_name){
  MSG("Usage: "<<prog_name<< " -l lumi [fb] -t truth_file.root -i in_file.root -s (to include systematics)");
}

int main(const int argc, char* const argv[]){
  char* in_fname=NULL;
  char* truth_fname=NULL;
  bool do_syst=false;
  int num_iter(0);
  double lumi=-1;
  int c;
  while((c = getopt(argc,argv,"st:i:l:")) != -1){
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
    case 'l':
      lumi=atof(optarg);
      break;
    default:
      abort();
    }
  }
  if(in_fname==NULL || truth_fname==NULL || !std::isfinite(lumi)){
    usage(argv[0]);
    exit(1);
  }

  setup_global_style();
  std::map<std::string,aesthetic> hist_styles;
  init_hist_styles(hist_styles);
  MSG("Input File: "<<in_fname);
  MSG("Truth File: "<<truth_fname);
  TTree* tree = retrieve<TTree>(truth_fname,"mini");
  TFile* reco_file = TFile::Open(in_fname);
  
  const char* variables[] = {"delta_r","jet_pt","jet_z",
			     //"jpsi_pt","jpsi_eta","tau1","tau2","tau3","tau32","tau21"
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
    TH1* base_hist = dynamic_cast<TH1D*>(reco_hist->Clone(name.c_str()));
    TH2* response_hist = mc_response(base_hist,tree,n_evts);
    num_iter = get_iterations(base_hist,response_hist,int(reco_hist->Integral()));
    TH1* unfolded = unfold(response_hist,reco_hist,num_iter,name);
    TH1* truth = make_normal_hist(base_hist,tree,"truth_"+name);
    style_hist(truth,hist_styles["signal"]);
    //dump_hist(truth);
    unfolded->GetYaxis()->SetTitle();
    fix_axis_labels(unfolded);
    if(std::string(base_hist->GetName())=="delta_r"){
      unfolded->SetNdivisions(508);
    }

    unfolded->GetXaxis()->SetTitle(base_hist->GetXaxis()->GetTitle());
    TCanvas canv("canv","canv",600,600);
    TLegend* leg = init_legend(0.65,0.7,0.9,0.9);
    if(std::string(base_hist->GetName())=="jet_z"){
      delete leg;
      leg = init_legend(0.2,0.5,0.6,0.75);
    } 
    int width=7;
    unfolded->SetLineWidth(width);
    unfolded->SetMarkerSize(0);
    unfolded->SetFillColor(TColor::GetColorTransparent(kBlack,0.4));
    unfolded->SetLineColor(TColor::GetColorTransparent(kBlack,0.4));
    unfolded->SetMarkerColor(TColor::GetColorTransparent(kBlack,0.4));
    // unfolded->SetLineWidth(15);
    TH1* draw_hist =unfolded->DrawCopy("e2");
    leg->AddEntry(draw_hist,"Stat + Syst  Error","lf");
    if(do_syst){
      //square peg, round hole. I'm pidgeon-holing num_iter into a string... what could go wrong
      char iter_str[20];
      snprintf(iter_str,20,"%d",num_iter);
      TH1* syst_err_hist = build_syst_err_hist(reco_hist,"total",iter_str,unfold_syst_err);
      TH1* splot_err_hist = nullptr;
      reco_file->GetObject((std::string(variables[i])+"_sig_tot_err").c_str(),splot_err_hist);
      if(splot_err_hist!=nullptr){
	MSG_DEBUG("Adding splot syst hist");
	//dump_hist(splot_err_hist);
	width-=2;
	unfolded->SetLineWidth(width);
	unfolded->Add(splot_err_hist);
	unfolded->DrawCopy("e2 same");
      }
      width-=2;
      TH1* unfold_err_hist = build_unfold_err_hist(unfolded,response_hist,num_iter,name);
      //dump_hist(unfold_err_hist);
      unfolded->Add(unfold_err_hist);
      unfolded->Add(syst_err_hist);
      unfolded->SetLineWidth(width);
      // unfolded->DrawCopy("e2 same");
    }
    unfolded->SetFillColor(kBlack);
    unfolded->SetLineColor(kBlack);
    unfolded->SetMarkerColor(kBlack);
    unfolded->SetMarkerStyle(kFullDotLarge);
    unfolded->SetMarkerSize(0.75);
    
    truth->SetFillColor(0);
    truth->SetLineWidth(2);
    truth->Scale(unfolded->Integral()/truth->Integral());
    truth->DrawCopy("H same");
    leg->AddEntry(truth,"MC Truth","l");
    leg->AddEntry(unfolded,"Unfolded Data","lp");
    unfolded->DrawCopy("e0 x0 same");
    
    double udraw_max = unfolded->GetBinContent(unfolded->GetMaximumBin())
      + unfolded->GetBinError(unfolded->GetMaximumBin());
    double tdraw_max = truth->GetBinContent(truth->GetMaximumBin())
      + truth->GetBinError(truth->GetMaximumBin());
    draw_hist->SetMaximum(1.2*std::max(udraw_max,tdraw_max));
    leg->Draw();
    add_atlas_badge(canv,0.2,0.88,lumi);
    canv.SaveAs((std::string(variables[i])+"_unfolded.pdf").c_str());
  }
  reco_file->Close();
  
  return 0;
}
