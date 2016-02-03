#include "TTree.h"
#include "TROOT.h"
#include "TColor.h"
#include "TStyle.h"
#include "TH1D.h"
#include "TH1.h"
#include "TCanvas.h"
#include "TLegend.h"

#include "color.hh"
#include "math.hh"
#include "root-sugar.hh"
#include "histo-meta-data.hh"
#include "histo-utils.hh"
#include "histo-style.hh"
#include "plot-utils.hh"
#include "AtlasStyle.hh"

void usage(const char* name){
  MSG("Usage: "<< name << " [DSID.variation.hist.root]");
  MSG("Prints histograms outputted by make-systematics-plots, input name matters! ");
}
void scale_errors(TH1D* hist){
  double err(0);
  double content(0);
  for(int i=0; i < hist->GetNbinsX(); ++i){
    err=hist->GetBinError(i);
    content=hist->GetBinContent(i);
    // MSG_DEBUG("err: "<<err<< " content: "<<content);
    hist->SetBinError(i,0);
    hist->SetBinContent(i,content > 0 ? err/content : 0);
  }
}
void add_err(TH1D* hista, TH1D* histb){
  if(hista->GetNbinsX()!=histb->GetNbinsX()){
    MSG_ERR("Bin size mismatch: "<<hista->GetNbinsX()<<" vs "<<histb->GetNbinsX());
    return;
  }
  num_err a; num_err b;
  for(int i=0; i < hista->GetNbinsX(); i++){
    a.val=0; a.err=hista->GetBinError(i);
    b.val=0; b.err=histb->GetBinError(i);
    // MSG_DEBUG("a: "<<str_rep(a)<< " b: "<<str_rep(b)<<" a+b: "<<str_rep(add(a,b)));
    hista->SetBinError(i,add(a,b).err);
  }
}
bool has_non_zero_error(TH1* hist){
  // double tot_err=0;
  for(int i=1; i < hist->GetNbinsX(); i++){
    if(hist->GetBinError(i) != 0){
      return true;
    }
  }
  return false;
}
int main(const int argc, const char* argv[]){
  if(argc < 2){
    usage(argv[0]);
    return 0;
  }
  AtlasStyle style;
  style.SetAtlasStyle();
  std::map<std::string, TFile*> files;
  std::vector<std::string> parts;
  hist_book HistBook;
  init_hist_book(HistBook);
  std::vector<std::string> plots = map_keys(HistBook);
  for(int i=1; i < argc; i++ ){
    parts=split_string(argv[i],'.');
    files[parts.at(parts.size()-3)]=new TFile(argv[i]);
    // MSG_DEBUG(parts.at(parts.size()-3));
  }
  std::map<std::string,aesthetic> styles;
  std::vector<int> colors = qualitative(WARM,files.size());
  
  size_t idx=0;
  std::map<std::string,std::string> syst_leg;
  syst_leg["MuonEfficiencyUp"]="Muon Efficiency";
  syst_leg["MuonSmeared"]="Muon CB Resolution";
  syst_leg["MuonSmearedIDUp"]="Muon ID Resolution";
  syst_leg["MuonSmearedMSUp"]="Muon MS Resolution";
  syst_leg["MuonSmearedUp"]="Muon Scale";
  syst_leg["TrackZFilteredJets"]="Track Efficiency";
  syst_leg["TrackZRadialScaledUpJets"]="Track Radial Scale";
  syst_leg["TrackZScaledUpJets"]="Track Sagitta Scale";
  syst_leg["TrackZSmearedJets"]="Track Resolution";
  
  for(std::map<std::string,TFile*>::const_iterator f=files.begin(); f!=files.end();++f){
    MSG_DEBUG(f->first<<" => "<<colors.at(idx));
    styles[f->first]=hist_aes(syst_leg[f->first].c_str(),TColor::GetColorTransparent(colors.at(idx),0.4),1001,0);
    idx++;
  }
  
  for(std::vector<std::string>::const_iterator p=plots.begin(); p!=plots.end(); ++p){
    const std::string& plot = *p;
    MSG_DEBUG(plot);
    TH1D* tot_err=dynamic_cast<TH1D*>(HistBook[plot]->Clone((plot+"_tot_err").c_str()));
    TCanvas canv("canv","canv",1200,600);
    canv.Divide(2,1);
    // canv.SetLogy();
    double max(10);
    TLegend *leg = init_legend();
    for(std::map<std::string,TFile*>::iterator f=files.begin();
	f!=files.end(); ++f){
      TVirtualPad* pad = canv.cd(1);
      TH1D* hist = retrieve<TH1D>(f->second,(plot+"_syst").c_str());
      if(tot_err->GetEntries()==0){
	for(int i=0; i < hist->GetNbinsX(); i++){
	  tot_err->SetBinContent(i,hist->GetBinContent(i));
	  tot_err->SetBinError(i,0);
	}
      }
      add_err(tot_err,hist);
      style_hist(hist,styles[f->first]);
      add_to_legend(leg,hist,styles[f->first]);
      TH1D* nom = dynamic_cast<TH1D*>(hist->Clone("tmp"));
      nom->SetFillStyle(0);
      nom->SetLineColor(kBlack);
      hist->SetLineWidth(0);
      nom->Draw("HIST same");
      if(has_non_zero_error(hist)){
	hist->Draw("e2 same");
      }
      pad=canv.cd(2);
      pad->SetLogy();
      TH1D* rel_err = dynamic_cast<TH1D*>(hist->Clone((std::string(hist->GetName())+"_rel_err").c_str()));
      scale_errors(rel_err);
      Color_t cn=styles[f->first].color;
      rel_err->SetLineColor(TColor::GetColorTransparent(cn,1.0));
      rel_err->SetFillColor(TColor::GetColorTransparent(cn,1.0));
      rel_err->SetFillStyle(0);
      rel_err->SetLineWidth(2);
      rel_err->GetYaxis()->SetTitle("Relative Systematic Error");
      rel_err->SetMaximum(max);
      rel_err->Draw("H same");
    }
    aesthetic tot_aes = hist_aes("Total Syst Error",TColor::GetColorTransparent(kBlack,0.4),1001,0);
    TH1D* rel_err = dynamic_cast<TH1D*>(tot_err->Clone((std::string(tot_err->GetName())+"_rel_err").c_str()));
    scale_errors(rel_err);
    add_to_legend(leg,tot_err,tot_aes);
    rel_err->SetLineColor(TColor::GetColorTransparent(tot_aes.color,1.0));
    rel_err->SetFillColor(TColor::GetColorTransparent(tot_aes.color,1.0));
    rel_err->SetFillStyle(0);
    rel_err->SetLineWidth(2);
    canv.cd(2);
    rel_err->Draw("H same");
    style_hist(tot_err,tot_aes);
    canv.cd(1);
    tot_err->Draw("e2 same");
    
    canv.cd();
    leg->Draw();
    canv.SaveAs((plot+"_syst.pdf").c_str());
  }
  
}
