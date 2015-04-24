#include <vector>
#include <map>

#include "TTree.h"
#include "TF1.h"
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
num_err get_yield(RooAbsPdf* PDF, RooRealVar* var, const char* key){
  std::list<std::string> regions = var->getBinningNames();
  RooAbsReal* integral=NULL;
  num_err yield={0.,0.};
  double roo_yield(0.);
  RooArgList float_pars(*PDF->getParameters(RooArgSet(*var)));
  TF1* fn = PDF->asTF(RooArgList(*var),float_pars);
  //FIXME add the damn error integral part
  if(fn==NULL){
    MSG_ERR("Could not properly cast PDF to TF1 "<<PDF->GetName()<<" for var: "<<var->GetName());
    exit(-3);
  }
  // RooArgSet prod(PDF);
  for(std::list<std::string>::const_iterator r = regions.begin();
      r!=regions.end(); ++r){
    if(r->find(key)!=std::string::npos){
      MSG_DEBUG(*r);
      //this "idiom" is the most opaquely obvious way to get these values out
      yield.val+=fn->Integral(var->getMin(r->c_str()),
			      var->getMax(r->c_str()));
      // MSG_DEBUG(var->getMax(r->c_str())<<" "<<var->getMin(r->c_str()));
      integral=PDF->createIntegral(*var,RooFit::Range(r->c_str()));
      roo_yield += integral->getVal();
    }
  }
  MSG_DEBUG("RooFit: "<<roo_yield<<" TF1: "<<yield.val);
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
		   const char* prefix, const num_err sf){
  char cut_expr[500];
  snprintf(cut_expr,sizeof cut_expr/sizeof *cut_expr,
	   "(%s) && (%s)",
	   make_cut_expr(sb_regions,"SB").c_str(),
	   make_cut_expr(sig_regions,"Sig").c_str());
  TH1* result = make_normal_hist(base_hist, tree, base_hist->GetName(),
				 cut_expr,prefix);
  result->Scale(sf.val);
  return result;
}
void style_bkg_hist(TH1* hist,Int_t color){
  hist->SetFillStyle(1001);
  hist->SetFillColor(color);
  hist->SetLineColor(color);
}
static void ensure_sumw2(TH1* hist){
  if(hist->GetSumw2N()==0){
    hist->Sumw2();
  }
}
void print_sbs_stack(TTree* tree, TH1* base_hist, const char* suffix,
		     const std::list<std::string> mass_regions, 
		     const std::list<std::string> tau_regions,
		     const std::list<std::string> psi_regions,
		     const num_err mass_stsR, const num_err np_frac, 
		     const num_err psi_stsR){
  //This covers:
  // S = S' + R*SB(mass) + F*SB(tau) + Q*S(psi_m)
  // S  == Observed signal in tau/mass signal region
  // S' == Real prompt J/\psi contribution
  // R  == Signal-to-Sideband ratio from mass fit
  // SB(mass) == Mass side band hist
  // F  == non-prompt fraction from fit
  // SB(tau) == Tau sideband hist
  // Q  == Branching fractions to estimate \psi(2S) contamination
  // S(psi_m) = Signal of \psi(2S)

  //BR(J/\psi->\mu\mu)=0.05961
  ///1/BR(\psi(2S)->J/\psi\pi\pi)
  const double jpsi_pi_br(1/0.3445);
  TLegend leg = *init_legend();
  // Signal Hist
  const std::string signal_cut_expr = make_cut_expr(mass_regions,"Sig")+ " && "
    + make_cut_expr(tau_regions,"Sig");
  TH1* sig_hist = make_normal_hist(base_hist, tree, base_hist->GetName(),
				   signal_cut_expr.c_str(),"_stk_sig");

  // Mass SB Hist
  TH1* comb_hist =  make_bkg_hist(base_hist,tree,mass_regions,tau_regions,"mass_stk_sb",mass_stsR);
  // Tau SB Hist
  TH1* nonprompt_hist = make_bkg_hist(base_hist,tree,tau_regions,mass_regions,"tau_stk_sb",np_frac);

  // Psi Mass Hist
  TH1* psi_hist = make_normal_hist(base_hist,tree,base_hist->GetName(),
				   (signal_cut_expr + "&& "+ make_cut_expr(psi_regions,"Sig")).c_str(),
				   "_stk_psi_sig");
  TH1* psi_sb_hist = make_normal_hist(base_hist,tree,base_hist->GetName(),
				      make_cut_expr(psi_regions, "SB").c_str(),"_stk_psi_sb");
  psi_sb_hist->Scale(-psi_stsR.val);
  psi_hist->Add(psi_sb_hist);
  psi_hist->Scale(jpsi_pi_br);

  THStack stack("sbs_stack",base_hist->GetTitle());
  stack.SetHistogram((TH1*)base_hist->Clone((std::string("stack_sbs")+base_hist->GetName()).c_str()));
  sig_hist->SetLineColor(kBlack);
  sig_hist->SetFillStyle(0);
  
  style_bkg_hist(comb_hist,12);
  style_bkg_hist(nonprompt_hist,14);
  style_bkg_hist(psi_hist,16);
  stack.Add(psi_hist);
  stack.Add(nonprompt_hist);
  stack.Add(comb_hist); 
  leg.AddEntry(sig_hist,"Signal","l");
  leg.AddEntry(comb_hist,"Comb. Background","f");
  leg.AddEntry(nonprompt_hist,"Non-prompt Background","f");
  leg.AddEntry(psi_hist,"#psi(2S) Background","f");

  TH1* sig_final = dynamic_cast<TH1*>(sig_hist->Clone((std::string("sf_")+base_hist->GetName()).c_str()));
  sig_final->Add(comb_hist,-1);
  sig_final->Add(nonprompt_hist,-1);
  sig_final->Add(psi_hist,-1);
  TCanvas c1("Canvas","Canvas",1200,600);
  c1.Divide(2,1);
  c1.cd(1);
  sig_hist->Draw("H");
  stack.Draw("H same");
  leg.Draw();
  c1.cd(2);
  sig_final->Draw("H");
  char outname[256];
  snprintf(outname,256,"%s%s",base_hist->GetName(),suffix);
  c1.SaveAs(outname);
}
RooAbsPdf* find_component(RooAbsPdf* PDF,const char* name){
  TIterator *iter = PDF->getComponents()->createIterator();
  RooAbsArg* var = NULL;
  RooAbsPdf* comp = NULL;
  while((var = dynamic_cast<RooAbsArg*>(iter->Next()))){
    std::string var_name(var->GetName());
    if(var_name.find(name)!=std::string::npos){
      comp=dynamic_cast<RooAbsPdf*>(var);
    }
  }
  if(comp == NULL){
    MSG_ERR("Could not find \""<<name<<"\" in the model PDF!");
  }
  return comp;
}
void do_sbs(const char** variables, const size_t n_vars,
	    TTree* tree, RooAbsPdf* model, 
	    RooRealVar* mass, RooRealVar* tau,
	    RooRealVar* psi_m, num_err psi_stsR, 
	    const char* suffix){
  std::map<std::string,TH1D*> HistBook;
  init_hist_book(HistBook);
  RooAbsPdf* np_mass_bkg = find_component(model,"NonPromptBkgMass");
  RooAbsPdf* p_mass_bkg = find_component(model,"PromptBkgMass");
  num_err mass_sig = add(get_yield(np_mass_bkg, mass,"Sig"),
			 get_yield(p_mass_bkg,mass,"Sig"));
  num_err mass_sb = add(get_yield(np_mass_bkg, mass,"SB"),
			get_yield(p_mass_bkg,mass,"SB"));
  num_err mass_stsR = div(mass_sig,mass_sb);

  RooAbsPdf* tau_sig = find_component(model,"NonPromptSigTau");
  num_err np_frac = div(get_yield(tau_sig,tau,"Sig"),get_yield(tau_sig,tau,"SB"));
  MSG_DEBUG("Non-prompt fraction: "<<str_rep(np_frac)<<" Combinatoric Fraction: "<<str_rep(mass_stsR)<<" Psi(2S) fraction: "<<str_rep(psi_stsR));
  for(size_t i=0; i < n_vars; i++){
    print_sbs_stack(tree,HistBook[variables[i]],suffix,
		    mass->getBinningNames(),
		    tau->getBinningNames(),
		    psi_m->getBinningNames(),
		    np_frac,
		    mass_stsR,
		    psi_stsR);
  }
}
