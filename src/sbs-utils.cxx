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
#include "RooWorkspace.h"
#include "RooPlot.h"
#include "RooHist.h"
#include "RooDataSet.h"
#include "RooStats/SPlot.h"

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
static TH1* make_np_syst_hist(TH1* base_hist, const TH1* tau_template, 
			      const std::list<std::string>& mass_regions, 
			      const std::list<std::string>& tau_regions){
  const std::string sig_expr = make_cut_expr(mass_regions,"Sig") + " && "+make_cut_expr(tau_regions,"Sig");
  const std::string tau_sb_expr = make_cut_expr(mass_regions,"Sig")+ " && "+make_cut_expr(tau_regions,"SB");
  TTree* np_tree = retrieve<TTree>("non_prompt.mini.root","mini");
  TH1* sig_hist = make_normal_hist(base_hist,np_tree,base_hist->GetName(),sig_expr.c_str(),"_np_sig_syst");
  TH1* sb_hist = make_normal_hist(base_hist,np_tree,base_hist->GetName(),tau_sb_expr.c_str(),"_np_sb_syst");
  sig_hist->Add(sb_hist,-1.0);
  sig_hist->Divide(sb_hist);
  double rel_err(0);
  for(int i=0; i < sb_hist->GetNbinsX(); i++){
    rel_err=sig_hist->GetBinContent(i);
    sig_hist->SetBinError(i,rel_err*tau_template->GetBinContent(i));
    sig_hist->SetBinContent(i,0);
  }
  return sig_hist;
}
static TH1* make_comb_syst_hist(TH1* base_hist, TTree* tree, 
				const std::list<std::string>& mass_regions, 
				const std::list<std::string>& tau_regions,
				const num_err sf){
  std::list<std::string> sb1;
  std::list<std::string> sb2;
  for(std::list<std::string>::const_iterator it = mass_regions.begin();
      it != mass_regions.end(); ++it){
    if(it->find("SB")!=std::string::npos){
      sb1.size() == 0 ? sb1.push_back(*it) : sb2.push_back(*it);
    }
  }
  const std::string sb1_expr = make_cut_expr(sb1,"SB") + " && "+make_cut_expr(tau_regions,"Sig");
  const std::string sb2_expr = make_cut_expr(sb2,"SB") + " && "+make_cut_expr(tau_regions,"Sig");

  TH1* sb1_hist = make_normal_hist(base_hist,tree,base_hist->GetName(),sb1_expr.c_str(),"cmb_syst_sb1");
  TH1* sb2_hist = make_normal_hist(base_hist,tree,base_hist->GetName(),sb2_expr.c_str(),"cmb_syst_sb2");
  double sb1_bc(0),sb2_bc(0);
  for(int i=0; i < sb1_hist->GetNbinsX(); i++){
    sb1_bc=sb1_hist->GetBinContent(i);
    sb2_bc=sb2_hist->GetBinContent(i);
    sb1_hist->SetBinError(i,0.5*fabs(sb1_bc-sb2_bc)*sf.val);
    sb1_hist->SetBinContent(i,0);
  }
  return sb1_hist;
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
void rooplot_to_hist(const RooPlot* input,TH1* hist){
  hist->Print();
  input->Print();
  RooHist* roo_hist = input->getHist();
  double* y=roo_hist->GetY();
  double* y_err=roo_hist->GetEYhigh();
  if(input->GetNbinsX()!=hist->GetNbinsX()){
    MSG_ERR("ERROR: Refusing to copy RooPlot to TH1 with mismatching bins");
  }
  for(int i =0; i < input->GetNbinsX(); i++){
    // MSG_DEBUG("i: "<<i << " y: "<<y[i]<<" +/-"<< y_err[i]);
    hist->SetBinContent(i+1,y[i]);
    hist->SetBinError(i+1,y_err[i]);
  }
}
TH1* print_splot_stack(TTree* tree, TH1* base_hist, const char* suffix,
		       const double lumi,RooWorkspace* wkspc){
  const std::string plot_name(base_hist->GetName());
  RooAbsPdf& model = *wkspc->pdf("model");
  RooAbsPdf& Signal = *wkspc->pdf("ext_sig");
  RooAbsPdf& Background = *wkspc->pdf("ext_bkg");
  RooRealVar& mass = *wkspc->var("jpsi_m");
  RooRealVar& tau  = *wkspc->var("jpsi_tau");
  RooRealVar& nsig = *wkspc->var("nsig");
  RooRealVar& nbkg = *wkspc->var("nbkg");
  RooRealVar interest_var(base_hist->GetName(),base_hist->GetName(),
			  base_hist->GetXaxis()->GetXmin(),
			  base_hist->GetXaxis()->GetXmax());
  RooDataSet* data=new RooDataSet("data","data",RooArgSet(mass,tau,interest_var),RooFit::Import(*tree));//dynamic_cast<RooDataSet*>(wkspc->data("data"));
  RooFIter iter = model.getVariables()->fwdIterator();
  RooRealVar* var = NULL;
  while((var=dynamic_cast<RooRealVar*>(iter.next()))){
    var->setConstant();
  }
  RooStats::SPlot sData("sData","SPlot Dataset ", *data, &model,RooArgList(nsig,nbkg));
  TCanvas canv("canv","SPlot diagnostic Canvas",600,600);
  RooDataSet data_signal(data->GetName(),data->GetTitle(),data,*data->get(),0,"nsig_sw");
  // RooDataSet data_background(data->GetName(),data->GetTitle(),data,*data->get(),0,"nbkg_sw");
  RooPlot* sig_var_frame = new RooPlot(base_hist->GetName(),base_hist->GetTitle(),interest_var,
				       base_hist->GetXaxis()->GetXmin(),base_hist->GetXaxis()->GetXmax(),
				       base_hist->GetNbinsX());
  data_signal.plotOn(sig_var_frame,RooFit::DataError(RooAbsData::SumW2));
  sig_var_frame->Draw();
  char outname[256];
  snprintf(outname,256,"%s_splot%s",base_hist->GetName(),suffix);
  add_atlas_badge(canv,0.2,0.9,lumi);
  canv.SaveAs(outname);
  TH1* sig_final = dynamic_cast<TH1*>(base_hist->Clone(("sf_"+plot_name).c_str()));
  rooplot_to_hist(sig_var_frame,sig_final);
  return sig_final;
}
TH1* print_sbs_stack(TTree* tree, TH1* base_hist, const char* suffix,
		     std::map<std::string,sb_info> sep_var_info, 
		     const double lumi){
  const std::list<std::string>& mass_regions=sep_var_info["mass"].regions; 
  const std::list<std::string>& tau_regions=sep_var_info["tau"].regions;
  // const std::list<std::string>& psi_regions=sep_var_info["psi_m"].regions;
  const num_err& mass_stsR=sep_var_info["mass"].sts_ratio;
  const num_err& np_frac=sep_var_info["tau"].sts_ratio;

  //This covers:
  // S = S' + R*SB(mass) + F*SB(tau)
  // S  == Observed signal in tau/mass signal region
  // S' == Real prompt J/\psi contribution
  // R  == Signal-to-Sideband ratio from mass fit
  // SB(mass) == Mass side band hist
  // F  == non-prompt fraction from fit
  // SB(tau) == Tau sideband hist

  TLegend* leg = NULL;
  const std::string plot_name(base_hist->GetName());
  if(plot_name=="jet_z"){
    leg = init_legend(0.2,0.4,0.6,0.75);
  } 
  else {
    leg = init_legend();
  }
  
  std::map<std::string,aesthetic> styles;
  init_hist_styles(styles);
  // Signal Hist
  const std::string signal_cut_expr = make_cut_expr(mass_regions,"Sig")+ " && "
    + make_cut_expr(tau_regions,"Sig");
  TH1* sig_hist = make_normal_hist(base_hist, tree, plot_name.c_str(),
				   signal_cut_expr.c_str(),"_stk_sig");

  // Mass SB Hist 
  
  // There is not a bug here, mass_regions should be first for
  // make_bkg_hist for comb_hist and flipped for nonprompt_hist, see
  // further discussion in make_bkg_hist
  TH1* comb_hist =  make_bkg_hist(base_hist,tree,mass_regions,tau_regions,"mass_stk_sb",mass_stsR);
  TH1* comb_syst_hist = make_comb_syst_hist(base_hist,tree,mass_regions,tau_regions,mass_stsR);
  TH1* comb_stat_hist = dynamic_cast<TH1*>(comb_hist->Clone("comb_stat_hist"));
  add_err(comb_hist,comb_syst_hist);
  // Tau SB Hist
  TH1* nonprompt_hist = make_bkg_hist(base_hist,tree,tau_regions,mass_regions,"tau_stk_sb",np_frac);
  TH1* np_syst_hist = make_np_syst_hist(base_hist,nonprompt_hist,mass_regions,tau_regions);
  TH1* np_stat_hist = dynamic_cast<TH1*>(nonprompt_hist->Clone("np_stat_hist"));
  add_err(nonprompt_hist,np_syst_hist);

  TH1* sig_final = dynamic_cast<TH1*>(sig_hist->Clone(("sf_"+plot_name).c_str()));
  sig_final->Add(comb_hist,-1);
  sig_final->Add(nonprompt_hist,-1);

  // The rest of this function is printing the plot 
  style_hist(sig_hist,styles["data"]);

  make_transparent(styles["comb_bkg"],0.4);
  make_transparent(styles["non_prompt"],0.4);

  style_hist(comb_hist,styles["comb_bkg"]);
  style_hist(nonprompt_hist,styles["non_prompt"]);
  style_hist(comb_stat_hist,styles["comb_bkg"]);
  style_hist(np_stat_hist,styles["non_prompt"]);

  add_to_legend(leg,sig_hist,styles["data"]);
  add_to_legend(leg,comb_hist,styles["comb_bkg"]);
  add_to_legend(leg,nonprompt_hist,styles["non_prompt"]);
  
  TCanvas c1("Canvas","Canvas",600,600);
  if(plot_name.find("pt")!=std::string::npos){
    c1.SetLogy(true);
  }
  sig_hist->Draw("e0");
  leg->Draw();
  nonprompt_hist->DrawCopy("e2 same");
  np_stat_hist->Draw("e2 same");
  nonprompt_hist->SetFillStyle(0);
  nonprompt_hist->Draw("HIST same");
  TH1* comb_stack = dynamic_cast<TH1*>(comb_hist->Clone("comb_stack"));
  comb_stack->Add(nonprompt_hist);
  for(int i =0; i < comb_stack->GetNbinsX(); i++){
    comb_stack->SetBinError(i,comb_hist->GetBinError(i));
    comb_stat_hist->SetBinContent(i,comb_stack->GetBinContent(i));
  }
  comb_stack->DrawCopy("e2 same");
  comb_stat_hist->Draw("e2 same");
  comb_stack->SetFillStyle(0);
  comb_stack->Draw("HIST same");
  double sf = c1.GetLogy() ? 14 : 1.4;
  if(plot_name.find("eta")!=std::string::npos){
    sf = 1.6;
  }
  sig_hist->SetMaximum(sf*std::max(comb_stack->GetMaximum(),sig_hist->GetMaximum()));
  if(!c1.GetLogy()){
    sig_hist->SetMinimum(std::min(comb_stack->GetMinimum(),sig_hist->GetMinimum()));
  }
  sig_hist->Draw("e0 same");
  char outname[256];
  snprintf(outname,256,"%s_sbs_stk%s",base_hist->GetName(),suffix);
  add_atlas_badge(c1,0.2,0.9,lumi);
  c1.SaveAs(outname);
  // snprintf(outname,256,"%s_sbs_stk%s",base_hist->GetName(),".root");
  // c1.SaveAs(outname);
  return sig_final;
}
THStack* build_stack(TH1* base_hist, TLegend* leg, std::map<std::string,aesthetic> styles, 
		     const char* cut_expr){
  THStack* stack = new THStack(("pythia_stk_"+std::string(base_hist->GetName())).c_str(),"Stack");
  const char* samp_names[]={
    "208025.Pythia8B_AU2_CTEQ6L1_pp_Jpsimu20mu20_3PJ_1",
    "208027.Pythia8B_AU2_CTEQ6L1_pp_Jpsimu20mu20_3S1_1",
    "208028.Pythia8B_AU2_CTEQ6L1_pp_Jpsimu20mu20_3S1_8",
    "208026.Pythia8B_AU2_CTEQ6L1_pp_Jpsimu20mu20_3PJ_8",
    "208024.Pythia8B_AU2_CTEQ6L1_pp_Jpsimu20mu20_1S0_8"
  };
  TH1* tot_syst_err = dynamic_cast<TH1*>(base_hist->Clone((std::string(base_hist->GetName())+"_global_syst_err").c_str()));
  // tot_syst_err->Reset("ICES");
  for(int i =0; i < tot_syst_err->GetNbinsX(); i++){
    tot_syst_err->SetBinContent(i,0);
    tot_syst_err->SetBinError(i,0);
  }
  style_hist(tot_syst_err,styles["global_syst_err"]);
  add_to_legend(leg,tot_syst_err,styles["global_syst_err"]);
  for(size_t i=0; i < LEN(samp_names); i++){
    const std::string name(samp_names[i]);
    TH1* syst_err= build_syst_err_hist(base_hist,name,cut_expr);
    TTree* tree = retrieve<TTree>((name+".mini.root").c_str(),"mini");
    TH1* hist = make_normal_hist(base_hist,tree,base_hist->GetName(), cut_expr, name);
    stack->Add(hist);
    style_hist(hist,styles[name]);
    add_to_legend(leg, hist, styles[name]);
    add_err(tot_syst_err,syst_err);
  }
  stack->Add(tot_syst_err);
  return stack;
}

void print_pythia_stack(TH1* base_hist, TH1* signal, 
			const double lumi,const char* cut_expr,
			const char* suffix){
  TCanvas canv(("pythia_canv_"+std::string(base_hist->GetName())).c_str(),"Stack",600,600);
  if(std::string(base_hist->GetName()).find("pt")!=std::string::npos){
    canv.SetLogy(true);
  }
  TLegend* leg=NULL;
  if(std::string(base_hist->GetName())=="jet_z"){
    leg = init_legend(0.2,0.4,0.6,0.75);
  } 
  else {
    leg = init_legend();
  }
  std::map<std::string,aesthetic> styles;
  init_hist_styles(styles);
  
  THStack* stack = build_stack(base_hist,leg,styles,cut_expr);

  double N_sig = signal->Integral();
  TH1* tot_syst_err=(TH1*)stack->GetStack()->Last()->Clone("tot_syst_err");
  double N_MC = tot_syst_err->Integral();
  for(int i =0; i < tot_syst_err->GetNbinsX(); i++){
    tot_syst_err->SetBinError(i,0);
  }

  TH1* hist = NULL;
  for(int i = 0; i < stack->GetStack()->GetEntries(); i++){
    hist = dynamic_cast<TH1*>(stack->GetStack()->At(i));
    hist->Scale(N_sig/N_MC);
  }
  TIter next(stack->GetHists());
  while((hist=dynamic_cast<TH1*>(next()))){
    if(std::string(hist->GetName()).find("global_syst_err")!=std::string::npos){
      MSG("Removing: "<<hist->GetName());
      stack->RecursiveRemove(hist);
      break;
    }
  }
  tot_syst_err->Add(hist);
  tot_syst_err->Scale(N_sig/N_MC);

  stack->Draw("HIST");
  stack->GetXaxis()->SetTitle(signal->GetXaxis()->GetTitle());
  stack->GetYaxis()->SetTitle(signal->GetYaxis()->GetTitle());
  
  signal->Draw("e0 same");
  tot_syst_err->SetFillColor(TColor::GetColorTransparent(kBlack,0.4));
  tot_syst_err->SetLineColor(TColor::GetColorTransparent(kBlack,0.4));
  tot_syst_err->SetMarkerColor(TColor::GetColorTransparent(kBlack,0.4));
  tot_syst_err->Draw("e2 same");
  double s_max=stack->GetMaximum();
  double m_max=signal->GetMaximum();

  double sf = canv.GetLogy() ? 5 : 1.4;
  if(std::string(base_hist->GetName()).find("eta")!=std::string::npos){
    sf = 1.6;
  }
  stack->SetMaximum((std::max(s_max,m_max))*sf);
  if(!canv.GetLogy()){
    stack->SetMinimum(std::min(signal->GetMinimum(),stack->GetMinimum()));
  }
  leg->Draw();
  add_atlas_badge(canv,0.2,0.9,lumi);
  add_to_legend(leg,signal,styles["data"]);
  char outname[256];
  snprintf(outname,sizeof(outname)/sizeof(*outname),
	   "%s_sbs_p8%s",base_hist->GetName(),suffix);
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
