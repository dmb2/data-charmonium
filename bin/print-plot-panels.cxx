#include "root-sugar.hh"
#include "TCanvas.h"
#include "TROOT.h"
#include "TColor.h"
#include "TH1.h"
#include "TFile.h"
#include "TLatex.h"
#include "AtlasStyle.hh"
#include "color.hh"

#include <string>
#include <math.hh>

#define LEN(arr) ((sizeof arr)/(sizeof *arr))

static void style_err_hist(TH1* hist,int color){
  hist->SetFillColor(color);
  hist->SetMarkerStyle(kDot);
  hist->SetMarkerSize(0);
}
void print_summary(const char* var){
  TCanvas canv("canv","Canvas",1200,1200);
  const char* plot_types[]={"res_vtxz_prof","res_dif","res_prof"};
  const char* sub_systems[]={"LCTopoJets","MuonLCTopoJets","TrackZJets"};
  canv.Divide(3,3);
  size_t pad_pos=1;
  TLatex decorator;
  decorator.SetTextSize(0.10);
  const char* fmt_name="%s/%s_%s.root";
  char fname[500];
  for(size_t i=0; i < LEN(plot_types); i++){
    for(size_t j=0; j < LEN(sub_systems); j++){
      snprintf(fname,LEN(fname),fmt_name,
	       sub_systems[j], var, plot_types[i]);
      TCanvas* pad_canv = retrieve<TCanvas>(fname,("canv_"+std::string(var)).c_str());
      canv.cd(pad_pos);
      TIter next(pad_canv->GetListOfPrimitives());
      TH1* h = NULL;
      while((h = dynamic_cast<TH1*>(next()))){
	const std::string name(h->GetName());
	if(name.find("std")!=std::string::npos ||
	   name.find("sigma")!=std::string::npos){
	  style_err_hist(h,TColor::GetColorTransparent(kRed,0.3));
	  h->Draw("e3 same");
	}
	else if(name.find("mean")!=std::string::npos){
	  h->Draw("e1 same");
	}
	else{
	  h->Draw("COLZ");
	}
      }
      pad_pos++;
    }
  }
  canv.cd(1); decorator.DrawLatexNDC(0.4,0.8,"Calo Jets");
  canv.cd(2); decorator.DrawLatexNDC(0.4,0.8,"#mu + Calo Jets");
  canv.cd(3); decorator.DrawLatexNDC(0.4,0.8,"Track Jets");
  
  canv.SaveAs((std::string(var)+"_summary.pdf").c_str());
}

int main(const int argc, const char** argv){
  AtlasStyle style;
  style.SetAtlasStyle();
  heat_gradient(gStyle);
  const char* variables[] = {"jet_e","jet_eta","jet_pt","jet_z"};
  for(size_t i=0; i < LEN(variables); i++){
    print_summary(variables[i]);
  }
  return 0;
}
