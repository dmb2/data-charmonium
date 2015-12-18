#include "TTree.h"
#include "TROOT.h"
#include "TColor.h"
#include "TStyle.h"
#include "TH1D.h"
#include "TH1.h"
#include "TCanvas.h"
#include "TLegend.h"

#include "color.hh"
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
  for(int i=1; i <= hist->GetNbinsX(); ++i){
    err=hist->GetBinError(i);
    content=hist->GetBinContent(i);
    // MSG_DEBUG("err: "<<err<< " content: "<<content);
    hist->SetBinError(i,0);
    hist->SetBinContent(i,content > 0 ? err/content : 0);
  }
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
  for(std::map<std::string,TFile*>::const_iterator f=files.begin(); f!=files.end();++f){
    MSG_DEBUG(f->first<<" => "<<colors.at(idx));
    TColor* color = gROOT->GetColor(colors.at(idx));
    
    color->SetAlpha(0.4);
    styles[f->first]=hist_aes(f->first.c_str(),color->GetNumber(),1001,0);
    idx++;
  }
  
  for(std::vector<std::string>::const_iterator p=plots.begin(); p!=plots.end(); ++p){
    const std::string& plot = *p;
    MSG_DEBUG(plot);
    TCanvas canv("canv","canv",600,600);
    // canv.SetLogy();
    double max(10);
    TLegend *leg = init_legend();
    for(std::map<std::string,TFile*>::iterator f=files.begin();
	f!=files.end(); ++f){
      TH1D* hist = retrieve<TH1D>(f->second,(plot+"_syst").c_str());
      // scale_errors(hist);
      // hist->GetYaxis()->SetTitle("Relative Systematic Error");
      style_hist(hist,styles[f->first]);
      add_to_legend(leg,hist,styles[f->first]);
      // hist->SetMaximum(max);
      TH1D* nom = dynamic_cast<TH1D*>(hist->Clone("tmp"));
      nom->SetFillStyle(0);
      nom->SetLineColor(kBlack);
      hist->SetLineWidth(0);
      nom->Draw("HIST same");
      hist->Draw("e2 same");
    }
    leg->Draw();
    canv.SaveAs((plot+"_syst.pdf").c_str());
  }
  
}
