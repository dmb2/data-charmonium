#include <vector>
#include <map>

#include "TTree.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "THStack.h"
#include "TH1D.h"
#include "TColor.h"

#include "RooAbsPdf.h"
#include "RooRealVar.h"
#include "RooAbsReal.h"
#include "RooAbsArg.h"
#include "RooArgSet.h"

#include "sbs-utils.hh"
#include "root-sugar.hh"
#include "histo-utils.hh"
#include "histo-meta-data.hh"
#include "plot-utils.hh"
#include "fit-utils.hh"

void add_region(RooRealVar* var, const char* type, double min, double max){
  char name[400];
  snprintf(name,400,"%-3s %.4g < %s && %s < %.4g", type, min,
	   var->GetName(), var->GetName(), max);
  var->setRange(name,min,max);
}
double get_yield(RooAbsPdf* PDF, RooRealVar* var, const char* key){
  std::list<std::string> regions = var->getBinningNames();
  RooAbsReal* integral=NULL;
  double yield(0.);
  for(std::list<std::string>::const_iterator r = regions.begin();
      r!=regions.end(); ++r){
    if(r->find(key)!=std::string::npos){
      integral=PDF->createIntegral(*var,RooFit::Range(r->c_str()));
      yield += integral->getVal();
    }
  }
  return yield;
}
std::string make_cut_expr(const std::list<std::string>& regions, const std::string& key){
  std::string expr;
  for(std::list<std::string>::const_iterator r = regions.begin();
      r!=regions.end(); ++r){
    if(*r == ""){
      continue;
    }
    if(r->find(key)!=std::string::npos){
      expr = (expr.size() > 0) ? expr + " || " : "";
      expr += "(" + r->substr(4) + ")";
    }
  }
  return expr;
}
TH1* make_bkg_hist(TH1* base_hist, TTree* tree, 
		   const std::list<std::string>& sb_regions, 
		   const std::list<std::string>&  sig_regions,
		   const char* prefix, const double sf){
  char cut_expr[500]; //someday this will be too small
  snprintf(cut_expr,sizeof cut_expr/sizeof *cut_expr,
	   "(%s) && (%s)",
	   make_cut_expr(sb_regions,"SB").c_str(),
	   make_cut_expr(sig_regions,"Sig").c_str());
  TH1* result = make_normal_hist(base_hist, tree, base_hist->GetName(),
				 cut_expr,prefix);
  result->Scale(sf);
  return result;
}
void style_bkg_hist(TH1* hist,Int_t color){
  hist->SetFillStyle(1001);
  hist->SetFillColor(color);
  hist->SetLineColor(color);
}
void print_sbs_stack(TTree* tree, TH1* base_hist, const char* suffix,
		     const std::list<std::string> mass_regions, 
		     const std::list<std::string> tau_regions,
		     const double mass_stsR, const double np_frac
		     /*, const double psi_br*/){
  //Eventually this will cover:
  // S = S' + R*SB(mass) + F*SB(tau) + Q*S(psi_m)
  // S  == Observed signal in tau/mass signal region
  // S' == Real prompt J/\psi contribution
  // R  == Signal-to-Sideband ratio from mass fit
  // SB(mass) == Mass side band hist
  // F  == non-prompt fraction from fit
  // SB(tau) == Tau sideband hist
  // Q  == Branching fractions to estimate \psi(2S) contamination
  // S(psi_m) = Signal of \psi(2S)

  TCanvas c1("Canvas","Canvas",600,600);
  TLegend leg = *init_legend();
  

  // Signal Hist
  const std::string signal_cut_expr = make_cut_expr(mass_regions,"Sig")+ " && "
    + make_cut_expr(tau_regions,"Sig");
  TH1* sig_hist = make_normal_hist(base_hist, tree, base_hist->GetName(),
				   signal_cut_expr.c_str(),"_stk_sig");

  // Mass SB Hist
  TH1* mass_sb_hist =  make_bkg_hist(base_hist,tree,mass_regions,tau_regions,"mass_stk_sb",mass_stsR);
  // Tau SB Hist
  TH1* tau_sb_hist = make_bkg_hist(base_hist,tree,tau_regions,mass_regions,"tau_stk_sb",np_frac);

  // Psi Mass Hist
  /* ADD ME */
  //  TH1* psi_sig_hist = make_bkg_hist(base_hist,tree,psi_regions,"psi_stk_sig",psi_br);
  THStack stack("sbs_stack",base_hist->GetTitle());
  stack.SetHistogram((TH1*)base_hist->Clone((std::string("stack_sbs")+base_hist->GetName()).c_str()));
  sig_hist->SetLineColor(kBlack);
  sig_hist->SetFillStyle(0);
  
  style_bkg_hist(mass_sb_hist,12);
  style_bkg_hist(tau_sb_hist,14);
  //style_bkg_hist(psi_sig_hist,16);
  stack.Add(tau_sb_hist);
  stack.Add(mass_sb_hist); 
  // stack.Add(psi_sig_hist);
  sig_hist->Draw("H");
  stack.Draw("H same");

  leg.AddEntry(sig_hist,"Signal","l");
  leg.AddEntry(mass_sb_hist,"Comb. Background","f");
  leg.AddEntry(tau_sb_hist,"Non-prompt Background","f");
  //leg.AddEntry(psi_sig_hist,"#psi(2S) Background","f");
  leg.Draw();
  
  char outname[256];
  snprintf(outname,256,"%s%s",base_hist->GetName(),suffix);
  c1.SaveAs(outname);
}
void do_sbs(const char** variables, const size_t n_vars,
	    TTree* tree, RooAbsPdf* model, RooRealVar* mass, RooRealVar* tau,
	    const char* suffix){


  std::map<std::string,TH1D*> HistBook;
  init_hist_book(HistBook);

  TIterator *iter = model->getComponents()->createIterator();
  RooAbsArg* var = NULL;
  RooAbsPdf* bkg = NULL;
  while((var = dynamic_cast<RooAbsArg*>(iter->Next()))){
    std::string name(var->GetName());
    if(name.find("Background")!=std::string::npos){
      bkg=dynamic_cast<RooAbsPdf*>(var);
    }
  }
  if(bkg == NULL){
    MSG_ERR("Could not find \"Background\" in the model PDF!");
  }
  double sig_yield = get_yield(bkg, mass,"Sig");
  double sb_yield = get_yield(bkg, mass, "SB");
  // MSG_DEBUG("Calculated a Signal-to-Sideband ratio of: "<<sig_yield/sb_yield);
  for(size_t i=0; i < n_vars; i++){
    // MSG_DEBUG("Now subtracting: "<< variables[i]);
    print_sbs_stack(tree,HistBook[variables[i]],suffix,
		    mass->getBinningNames(),
		    tau->getBinningNames(),
		    sig_yield/sb_yield,
		    get_par_val(model->getVariables(),"NonPromptRatio"));
  }
}
