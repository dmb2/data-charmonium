#include "root-sugar.hh"
#include "TCanvas.h"
#include "TROOT.h"
#include "TFile.h"
#include "TLatex.h"
#include "AtlasStyle.hh"
#include "color.hh"

#include <string>

#define LEN(arr) ((sizeof arr)/(sizeof *arr))

void print_summary(const char* var){
  TCanvas canv("canv","Canvas",1200,1200);
  const char* plot_types[]={"response","res_dif","rel_res"};
  const char* sub_systems[]={"LCTopoJets","MuonLCTopoJets","TrackZJets"};
  canv.Divide(3,3);
  size_t pad_pos=1;
  TLatex decorator;
  decorator.SetTextSize(0.10);
  const char* fmt_name="%s/%s_nominal_%s.root";
  char fname[500];
  for(size_t i=0; i < LEN(plot_types); i++){
    for(size_t j=0; j < LEN(sub_systems); j++){
      snprintf(fname,LEN(fname),fmt_name,
	       sub_systems[j], var, plot_types[i]);
      TCanvas* pad_canv = retrieve<TCanvas>(fname,("canv_"+std::string(var)).c_str());
      canv.cd(pad_pos);
      pad_canv->GetListOfPrimitives()->First()->Draw("COLZ");
      // pad_canv->Draw();
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
