#include <iostream>
#include <cmath>
#include <cstdlib>
#include <map>
#include <vector>
#include <string>

#include "TROOT.h"
#include "TStyle.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TTree.h"
#include "TCanvas.h"
#include "TProfile.h"
#include "TLegend.h"

#include "histo-utils.hh"
#include "stack-utils.hh"
#include "plot-utils.hh"
#include "histo-meta-data.hh"
#include "color.hh"
#include "root-sugar.hh"

void usage(const char* name){
  MSG("Usage: "<< name << " -i input.mini.root ");
}

int main(const int argc, char* const argv[]){
  char* in_fname=nullptr;
  int c;
  while((c = getopt(argc,argv,"i:"))!= -1){
    switch(c){
    case 'i':
      in_fname = optarg;
      break;
    default:
      abort();
    }
  }
  if(in_fname==nullptr){
    usage(argv[0]);
    exit(1);
  }
  setup_global_style();
  TFile* file = TFile::Open(in_fname);
  TTree* tree = nullptr;
  file->GetObject("mini",tree);
  std::map<std::string,TH1D*> HistBook;
  init_hist_book(HistBook);
  std::map<std::string,std::string> leg_names;
  leg_names["filt"]="Efficiency";
  leg_names["smear"]="Resolution";
  leg_names["sdown"]="Scale";
  leg_names["rdown"]="Radial Scale";
    
  const char* plot_names[] = { "jet_pt","delta_r","jet_z",
			       "tau1","tau2","tau3",
			       "tau32","tau21"
  };
  const char* syst_names[] = {
    "filt","smear","sup","rup"
  };
  std::vector<std::string> plots(plot_names,plot_names + sizeof(plot_names)/sizeof(*plot_names));
  for(std::vector<std::string>::const_iterator p=plots.begin(); p!=plots.end(); ++p){
    const std::string& plot = *p;
    std::vector<TProfile*> to_draw;
    MSG_DEBUG(plot);
    TH1D* hist = HistBook[plot];
    TAxis* axis = hist->GetXaxis();
    TLegend* leg = init_legend(0.65,0.65,0.92,0.92);
    for(size_t i = 0; i < LEN(syst_names); i++){
      std::string syst = syst_names[i];
      std::string plot_name = syst+"_"+plot;
      TH2D* error = new TH2D(plot_name.c_str(),hist->GetTitle(),
			     axis->GetNbins(),axis->GetXmin(),axis->GetXmax(),
			     50,-100,200);
      std::string draw_str = "100*(jet_"+syst+"_pt - jet_pt )/jet_pt:"+plot+">>"+plot_name;
      size_t pos=syst.find("up");
      if(pos!=std::string::npos){
	std::string sup = syst;
	syst.replace(pos,2,"down");
	draw_str = "100*(jet_"+sup+"_pt - jet_"+syst+"_pt )/jet_pt:"+plot+">>"+plot_name;
	MSG_DEBUG(draw_str);
      }
      std::string cut_expr = "jet_"+syst+"_pt > 45";
      tree->Draw(draw_str.c_str(),cut_expr.c_str());
      TProfile* prof = error->ProfileX();
      prof->SetMarkerStyle(24+i);
      leg->AddEntry(prof,leg_names[syst].c_str(),"p");
      prof->GetYaxis()->SetTitle("% Error");
      prof->GetXaxis()->SetTitle(hist->GetXaxis()->GetTitle());
      to_draw.push_back(prof);
    }
    TCanvas canv("canv","canv",600,600);
    for(std::vector<TProfile*>::iterator it = to_draw.begin();
	it!=to_draw.end(); ++it){
      TProfile* prof = *it;
      prof->Draw("e1 x0 same");
      prof->SetMaximum(15);
      prof->SetMinimum(-15);
    }
    leg->Draw();
    add_atlas_badge(canv, 0.2,0.9,-1);
    canv.SaveAs((plot+"_syst_err.pdf").c_str());
  } 
  file->Close();
  return 0;
}
