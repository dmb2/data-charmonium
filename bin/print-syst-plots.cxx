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

void usage(const char* name){
  MSG("Usage: "<< name << " [DSID.variation.hist.root]");
  MSG("Prints histograms outputted by make-systematics-plots, input name matters! ");
}

int main(const int argc, const char* argv[]){
  if(argc < 2){
    usage(argv[0]);
    return 0;
  }
  setup_global_style();
  std::map<std::string, TFile*> files;
  std::string base_name;
  std::vector<std::string> parts;
  hist_book HistBook;
  init_hist_book(HistBook);
  std::vector<std::string> plots = map_keys(HistBook);
  for(int i=1; i < argc; i++ ){
    parts=split_string(argv[i],'.');
    base_name=parts.at(0);
    files[parts.at(parts.size()-3)]=new TFile(argv[i]);
    // MSG_DEBUG(parts.at(parts.size()-3));
  }
  MSG_DEBUG(base_name+".total.hist.root");
  TFile outFile((base_name+".total.hist.root").c_str(),"RECREATE");
  std::map<std::string,aesthetic> styles;
  std::vector<int> colors = qualitative(WARM,files.size());
  
  size_t idx=0;
  std::map<std::string,std::string> syst_leg;
  syst_leg["MuonEfficiencyUp"]="Muon Efficiency";
  syst_leg["MuonSmeared"]="Muon CB Resolution";
  syst_leg["MuonSmearedIDUp"]="Muon ID Resolution";
  syst_leg["MuonSmearedMSUp"]="Muon MS Resolution";
  syst_leg["MuonSmearedUp"]="Muon Scale";
  syst_leg["TrackZFilteredJPsiJets"]="Track Efficiency";
  syst_leg["TrackZRadialScaledUpJPsiJets"]="Track Radial Scale";
  syst_leg["TrackZScaledUpJPsiJets"]="Track Sagitta Scale";
  syst_leg["TrackZSmearedJPsiJets"]="Track Resolution";
  
  for(std::map<std::string,TFile*>::const_iterator f=files.begin(); f!=files.end();++f){
    MSG_DEBUG(f->first<<" => "<<colors.at(idx));
    styles[f->first]=hist_aes(syst_leg[f->first].c_str(),TColor::GetColorTransparent(colors.at(idx),0.4),1001,0);
    idx++;
  }
  
  for(std::vector<std::string>::const_iterator p=plots.begin(); p!=plots.end(); ++p){
    const std::string& plot = *p;
    MSG_DEBUG(plot);
    TH1D* tot_err=dynamic_cast<TH1D*>(HistBook[plot]->Clone((plot+"_tot_err").c_str()));
    TCanvas canv("canv","canv",1800,600);
    canv.Divide(3,1);
    // canv.SetLogy();
    double max(10);
    TLegend *leg = init_legend(0.1,0.1,0.9,0.9);
    leg->SetTextSize(0.05);
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
      add_to_legend(leg,rel_err,styles[f->first]);
      rel_err->Draw("H same");
    }
    tot_err->Write();
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
    canv.cd(3);
    leg->Draw();
    canv.SaveAs((plot+"_syst.pdf").c_str());
  }
  outFile.Close();
}
