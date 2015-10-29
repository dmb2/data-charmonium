#include <vector>
#include <algorithm>
#include <cmath>

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
num_err get_yield(RooAbsPdf* PDF, RooRealVar* var, const char* key,const double* covmat){
  std::list<std::string> regions = var->getBinningNames();
  num_err yield={0.,0.};
  RooArgList float_pars(*PDF->getParameters(RooArgSet(*var)));
  TF1* fn = PDF->asTF(RooArgList(*var),float_pars);
  if(fn==NULL){
    MSG_ERR("Could not properly cast PDF to TF1 "<<PDF->GetName()<<" for var: "<<var->GetName());
    exit(-3);
  }
  // RooArgSet prod(PDF);
  num_err tmp_val;
  for(std::list<std::string>::const_iterator r = regions.begin();
      r!=regions.end(); ++r){
    if(r->find(key)!=std::string::npos){
      // MSG_DEBUG(*r);
      //this "idiom" is the most opaquely obvious way to get these values out
      tmp_val.val=fn->Integral(var->getMin(r->c_str()),
			      var->getMax(r->c_str()));
      tmp_val.err=fn->IntegralError(var->getMin(r->c_str()),
				    var->getMax(r->c_str()),
				    NULL,
				    covmat);
      if(!std::isfinite(tmp_val.err)){
	MSG_ERR("WARNING: "<<str_rep(tmp_val)<<" has a spurious error, setting to 0");
	tmp_val.err=0;
      }
      // MSG_DEBUG(str_rep(yield)<<" + "<<str_rep(tmp_val));
      yield=add(yield,tmp_val);
      // MSG_DEBUG(" = "<<str_rep(yield));
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
static void ensure_sumw2(TH1* hist){
  if(hist->GetSumw2N()==0){
    hist->Sumw2();
  }
}
void scale_hist(TH1* hist, num_err scale_factor){
  // Scales hist and propagates error correctly
  ensure_sumw2(hist);
  num_err bin_content={0.,0.};
  switch(hist->GetDimension()){
  case(1):
    for(Int_t i = 0; i < hist->GetNbinsX(); i++){
      bin_content.val=hist->GetBinContent(i);
      bin_content.err=hist->GetBinError(i);
      bin_content=mul(bin_content,scale_factor);
      hist->SetBinContent(i,bin_content.val);
      hist->SetBinError(i,bin_content.err);
    }
    break;
  case(2):
    for(Int_t i = 0; i < hist->GetNbinsX(); i++){
      for(Int_t j = 0; j < hist->GetNbinsY(); j++){
	bin_content.val=hist->GetBinContent(i,j);
	bin_content.err=hist->GetBinError(i,j);
	bin_content=mul(bin_content,scale_factor);
	hist->SetBinContent(i,j,bin_content.val);
	hist->SetBinError(i,j,bin_content.err);
      }
    }
    break;
  case(3):
    for(Int_t i = 0; i < hist->GetNbinsX(); i++){
      for(Int_t j = 0; j < hist->GetNbinsY(); j++){
	for(Int_t k = 0; k < hist->GetNbinsY(); k++){
	  bin_content.val=hist->GetBinContent(i,j,k);
	  bin_content.err=hist->GetBinError(i,j,k);
	  bin_content=mul(bin_content,scale_factor);
	  hist->SetBinContent(i,j,k,bin_content.val);
	  hist->SetBinError(i,j,k,bin_content.err);
	}
      }
    }
    break;
  default:
    MSG_ERR("ERROR: scaling not implemented for: "<<hist->ClassName());
    return;
  }
}
TH1* make_bkg_hist(TH1* base_hist, TTree* tree, 
		   const std::list<std::string>& sb_regions, 
		   const std::list<std::string>&  sig_regions,
		   const char* prefix, const num_err sf){
  //Everytime I re-read this function I'm convinced I made a mistake.
  //Here's some info:

  //The idea is that for one type of background I make sidebands that
  //overlap with the signal region in the other separation variable
  //the notation I used in my notebook is (X) to indicate which parts
  //of the phase space that I'm selecting. For example:
  //
  // Bkg(Mass) = sf*SB(Mass)(X)Sig(Tau)
  // 
  // Bkg(Tau) = sf*SB(Tau)(X)Sig(Mass)

  char cut_expr[500];
  snprintf(cut_expr,sizeof cut_expr/sizeof *cut_expr,
	   "(%s) && (%s)",
	   make_cut_expr(sb_regions,"SB").c_str(),
	   make_cut_expr(sig_regions,"Sig").c_str());
  TH1* result = make_normal_hist(base_hist, tree, base_hist->GetName(),
				 cut_expr,prefix);
  scale_hist(result,sf);
  return result;
}
void style_bkg_hist(TH1* hist,Int_t color){
  hist->SetMarkerSize(0);
  hist->SetMarkerStyle(kDot);
  hist->SetFillStyle(1001);
  hist->SetFillColor(color);
  hist->SetLineColor(color);
}
TH1* print_sbs_stack(TTree* tree, TH1* base_hist, const char* suffix,
		     std::map<std::string,sb_info> sep_var_info, 
		     const double lumi){
  const std::list<std::string>& mass_regions=sep_var_info["mass"].regions; 
  const std::list<std::string>& tau_regions=sep_var_info["tau"].regions;
  const std::list<std::string>& psi_regions=sep_var_info["psi_m"].regions;
  const num_err& mass_stsR=sep_var_info["mass"].sts_ratio;
  const num_err& np_frac=sep_var_info["tau"].sts_ratio;
  // const num_err& psi_stsR=sep_var_info["psi_m"].sts_ratio;

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

  //TODO make this a num_err and usethe PDG errors for the numbers
  ///BR(\psi(2S)->J/\psi X)/BR(\psi(2S)->J/\psi\pi\pi)
  // const double jpsi_pi_br(0.609/0.3445);
  TLegend leg = *init_legend();
  std::map<std::string,aesthetic> styles;
  init_hist_styles(styles);
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
  /*
  TH1* psi_hist = make_normal_hist(base_hist,tree,base_hist->GetName(),
				   (signal_cut_expr + "&& "+ make_cut_expr(psi_regions,"Sig")).c_str(),
				   "_stk_psi_sig");
  TH1* psi_sb_hist = make_normal_hist(base_hist,tree,base_hist->GetName(),
				      (signal_cut_expr + "&& "+ make_cut_expr(psi_regions, "SB")).c_str(),"_stk_psi_sb");
  scale_hist(psi_sb_hist,psi_stsR);
  // psi_sb_hist->Scale(-psi_stsR.val);
  psi_hist->Add(psi_sb_hist,-1);
  psi_hist->Scale(jpsi_pi_br);
  */
  THStack stack("sbs_stack",base_hist->GetTitle());
  stack.SetHistogram((TH1*)base_hist->Clone((std::string("stack_sbs")+base_hist->GetName()).c_str()));
  sig_hist->SetLineColor(kBlack);
  sig_hist->SetFillStyle(0);
  
  style_hist(comb_hist,styles["comb_bkg"]);
  style_hist(nonprompt_hist,styles["non_prompt"]);
  // style_hist(psi_hist,styles["psi_bkg"]);
  
  add_to_legend(&leg,sig_hist,styles["periodA"]);
  add_to_legend(&leg,comb_hist,styles["comb_bkg"]);
  add_to_legend(&leg,nonprompt_hist,styles["non_prompt"]);
  // add_to_legend(&leg,psi_hist,styles["psi_bkg"]);

  // stack.Add(psi_hist);
  stack.Add(nonprompt_hist);
  stack.Add(comb_hist); 

  TCanvas c1("Canvas","Canvas",600,600);
  stack.Draw("H e1");
  sig_hist->Draw("e0 same");
  stack.SetMaximum(1.2*std::max(stack.GetMaximum(),sig_hist->GetMaximum()));
  leg.Draw();
  char outname[256];
  snprintf(outname,256,"%s_sbs_stk%s",base_hist->GetName(),suffix);
  add_atlas_badge(c1,0.2,0.9,lumi,INTERNAL);
  c1.SaveAs(outname);
  snprintf(outname,256,"%s_sbs_stk%s",base_hist->GetName(),".root");
  c1.SaveAs(outname);

  TH1* sig_final = dynamic_cast<TH1*>(sig_hist->Clone((std::string("sf_")+base_hist->GetName()).c_str()));
  double integral(0.);
  double int_err(0.);
  integral=sig_final->IntegralAndError(0,sig_final->GetNbinsX(),int_err);
  num_err signal_yield={.val=integral,.err=int_err};
  integral=comb_hist->IntegralAndError(0,comb_hist->GetNbinsX(),int_err);
  num_err comb_yield={.val=integral,.err=int_err};
  integral=nonprompt_hist->IntegralAndError(0,nonprompt_hist->GetNbinsX(),int_err);
  num_err nonprompt_yield={.val=integral,.err=int_err};
  MSG_DEBUG("| Total Yield | "<<str_rep(signal_yield)<<"| "<<str_rep(div(signal_yield,signal_yield))<<" |");
  MSG_DEBUG("| Combinatoric | "<<str_rep(comb_yield)<<"| "<<str_rep(div(comb_yield,signal_yield))<<" |");
  MSG_DEBUG("| Non-Prompt | "<<str_rep(nonprompt_yield)<<"| "<<str_rep(div(nonprompt_yield,signal_yield))<<" |");
  sig_final->Add(comb_hist,-1);
  sig_final->Add(nonprompt_hist,-1);
  // sig_final->Add(psi_hist,-1);
  return sig_final;
}
THStack* build_stack(TH1* base_hist, TLegend* leg, std::map<std::string,aesthetic> styles, 
		     const char* cut_expr){
  THStack* stack = new THStack(("pythia_stk_"+std::string(base_hist->GetName())).c_str(),"Stack");
  const char* samp_names[]={
    "208024.Pythia8B_AU2_CTEQ6L1_pp_Jpsimu20mu20_1S0_8",
    "208025.Pythia8B_AU2_CTEQ6L1_pp_Jpsimu20mu20_3PJ_1",
    "208026.Pythia8B_AU2_CTEQ6L1_pp_Jpsimu20mu20_3PJ_8",
    "208027.Pythia8B_AU2_CTEQ6L1_pp_Jpsimu20mu20_3S1_1",
    "208028.Pythia8B_AU2_CTEQ6L1_pp_Jpsimu20mu20_3S1_8"
  };
  char fname[500];
  for(size_t i=0; i < LEN(samp_names); i++){
    const std::string name(samp_names[i]);
    snprintf(fname,LEN(fname),"%s.mini.root",name.c_str());
    TTree* tree = retrieve<TTree>(fname,"mini");
    TH1* hist=dynamic_cast<TH1*>(base_hist->Clone((name+base_hist->GetName()+"_p8_stk").c_str()));
    draw_histo(tree,base_hist->GetName(),hist->GetName(),cut_expr);
    stack->Add(hist);
    style_hist(hist,styles[name]);
    add_to_legend(leg, hist, styles[name]);
  }
  return stack;
}
void print_pythia_stack(TH1* base_hist, TH1* signal, 
			const double lumi,const char* cut_expr,
			const char* suffix){
  TCanvas canv(("pythia_canv_"+std::string(base_hist->GetName())).c_str(),"Stack",600,600);
  if(std::string(base_hist->GetName())=="jet_pt"){
    canv.SetLogy(true);
  }
  TLegend leg = *init_legend();
  std::map<std::string,aesthetic> styles;
  init_hist_styles(styles);
  signal->SetMaximum(1.2*signal->GetMaximum());
  
  THStack* stack = build_stack(base_hist,&leg,styles,cut_expr);
  stack->Draw("H e1");
  signal->Draw("e0 same");
  double s_max=stack->GetStack()!=NULL ? ((TH1*)stack->GetStack()->Last())->GetMaximum() : 0.;
  double m_max=signal->GetMaximum();
  // MSG_DEBUG("Stack: "<<s_max<<" Master: "<<m_max);
  signal->SetMaximum((s_max > m_max ? s_max : m_max)*1.2);

  leg.Draw();
  add_atlas_badge(canv,0.2,0.9,lumi,INTERNAL);
  add_to_legend(&leg,signal,styles["periodA"]);
  char outname[256];
  snprintf(outname,sizeof(outname)/sizeof(*outname),
	   "%s_sbs_p8%s",base_hist->GetName(),suffix);
  canv.SaveAs(outname);
  snprintf(outname,sizeof(outname)/sizeof(*outname),
	   "%s_sbs_p8%s",base_hist->GetName(),".root");
  canv.SaveAs(outname);
}

RooAbsPdf* find_component(RooAbsPdf* PDF,const char* name){
  TIterator *iter = PDF->getComponents()->createIterator();
  RooAbsArg* var = NULL;
  RooAbsPdf* comp = NULL;
  while((var = dynamic_cast<RooAbsArg*>(iter->Next()))){
    std::string var_name(var->GetName());
    if(var_name==std::string(name)){
      comp=dynamic_cast<RooAbsPdf*>(var);
    }
  }
  if(comp == NULL){
    MSG_ERR("Could not find \""<<name<<"\" in the model PDF!");
  }
  return comp;
}
