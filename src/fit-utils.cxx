#include "fit-utils.hh"
#include "root-sugar.hh"

#include "RooFit.h"
#include "RooDataSet.h"
#include "RooRealVar.h"
#include "RooGaussian.h"
#include "RooGenericPdf.h"
#include "RooChebychev.h"
#include "RooPolynomial.h"
#include "RooGaussModel.h"
#include "RooAddPdf.h"
#include "RooPlot.h"
#include "RooFitResult.h"
#include "RooProdPdf.h"
#include "RooDecay.h"
#include "RooRealVar.h"
#include "RooFitResult.h"

#include "TCanvas.h"
#include "TLegend.h"
RooAddPdf* build_signal(RooRealVar* mass, RooRealVar* tau, RooGaussModel* tau_uncert){
  // PROMPT
  // -->Mass
  RooRealVar* mean_m = new RooRealVar("mean_m","Mean",JPSIMASS,JPSIMASS-0.02,JPSIMASS+0.02);
  RooRealVar* sigma_m = new RooRealVar("sigma_m","Width",0.005,0.0,0.10);
  RooRealVar *PromptFrac = new RooRealVar("PromptSigFrac","Fraction of prompt events",0.5,0.0,1.0);
  RooGaussian *prompt_mass = new RooGaussian("PromptSigMass","Prompt Mass PDF",*mass,*mean_m,*sigma_m);
  // -->Tau
  RooRealVar* mean_t = dynamic_cast<RooRealVar*>(tau_uncert->getVariables()->find("mean_t"));
  RooRealVar* sigma_t = dynamic_cast<RooRealVar*>(tau_uncert->getVariables()->find("sigma_t"));
  RooGaussian* prompt_tau = new RooGaussian("PromptTauSig","Prompt Tau Signal",*tau,*mean_t,*sigma_t);
  // RooGaussModel*& prompt_tau = tau_uncert;
  // Mass*Tau
  RooProdPdf* prompt_sig = new RooProdPdf("PromptSig","Prompt Signal PDF",RooArgSet(*prompt_mass,*prompt_tau));
  
  // NON-PROMPT
  // -->Mass
  RooGaussian *nonprompt_mass = new RooGaussian("NonPromptSigMass","Non-Prompt Mass PDF",*mass,*mean_m,*sigma_m);
  // -->Tau
  RooRealVar* lifetime = new RooRealVar("lifetime","Fitted Lifetime",1.53,0.0,2.0);
  RooDecay* nonprompt_tau = new RooDecay("NonPromptSigTau","Non-Prompt Signal Tau PDF",*tau,*lifetime,*tau_uncert,RooDecay::SingleSided);
  // Mass*Tau
  RooProdPdf* nonprompt_sig = new RooProdPdf("NonPromptSig","Non-Prompt Signal PDF",RooArgSet(*nonprompt_mass,*nonprompt_tau));

  // nonprompt_sig + prompt_sig
  return new RooAddPdf("Signal","Signal Model", RooArgSet(*prompt_sig,*nonprompt_sig),*PromptFrac);
}
RooGenericPdf* build_mass_bkg(const char* name, RooRealVar* mass,double ptcl_mass){
  //ptcl_mass is the approximate mass of the resonance of interest

  const char* formula_fmt = "1+(5.0*(@0-%.4g))*@1+((5.0*(@0-%.4g))*(5.0*(@0-%.4g)))*@2";
  char formula[100];
  snprintf(formula,sizeof(formula)/sizeof(*formula),formula_fmt,ptcl_mass,ptcl_mass,ptcl_mass);
  RooRealVar* c0 = new RooRealVar((std::string(name)+"c0").c_str(),"Bkg Coeff c0",0.3, 0.0,1.0);
  RooRealVar* c1 = new RooRealVar((std::string(name)+"c1").c_str(),"Bkg Coeff c1",0.0,-3.0,3.0);
  
  return new RooGenericPdf(name, "Mass Background",formula,RooArgList(*mass,*c0,*c1));
}

RooAddPdf* build_background(RooRealVar* mass, RooRealVar* tau,RooGaussModel* tau_uncert){
  // PROMPT
  // -->Mass
  RooRealVar *PromptFrac = new RooRealVar("BkgPromptFrac","Fraction of prompt events",0.5,0.0,1.0);
  RooRealVar* Pc1 = new RooRealVar("Pc1","Slope", -0.27, -0.5,0.5);
  RooPolynomial *prompt_mass = new RooPolynomial("PromptBkgMass","Mass Background",*mass,RooArgList(*Pc1));
  // RooGenericPdf* prompt_mass = build_mass_bkg("PromptBkgMass",mass,JPSIMASS);
  // -->Tau
  // RooRealVar* mean_t = dynamic_cast<RooRealVar*>(tau_uncert->getVariables()->find("mean_t"));
  // RooRealVar* sigma_t = dynamic_cast<RooRealVar*>(tau_uncert->getVariables()->find("sigma_t"));
  // RooGaussian *prompt_gauss = new RooGaussian("prompt_gauss","Prompt Tau component",*tau,*mean_t,*sigma_t);

  //lt -> lifetime
  RooRealVar* bkg_prompt_lt = new RooRealVar("bkg_prompt_lt","Prompt Bkg Lifetime",0.15,0.0,3.0);
  //prompt_decay
  RooDecay* prompt_tau = new RooDecay("PromptBkgTau","Prompt Background Decay",*tau,*bkg_prompt_lt,*tau_uncert,RooDecay::Flipped);
  // RooRealVar *prompt_ratio = new RooRealVar("PromptRatio","Ratio between background decays",0.25,0.0,1.0);
  // RooAddPdf* prompt_tau = new RooAddPdf("PromptBkgTau","Prompt Tau Background", RooArgSet(*prompt_gauss,*prompt_decay),*prompt_ratio);

  // Mass*Tau
  RooProdPdf *prompt_bkg = new RooProdPdf("PromptBkg","Prompt Background",RooArgSet(*prompt_mass,*prompt_tau));
  // NON-PROMPT
  // -->Mass
  //RooGenericPdf* nonprompt_mass = build_mass_bkg("NonPromptBkgMass",mass,JPSIMASS);
  RooRealVar* NPc1 = new RooRealVar("NPc1","Slope", -0.27, -0.5,0.5);
  RooPolynomial *nonprompt_mass = new RooPolynomial("NonPromptBkgMass","Mass Background",*mass,RooArgList(*NPc1));
  // -->Tau
  // RooRealVar *nonprompt_ratio = new RooRealVar("NonPromptRatio","Ratio between background decays",0.25,0.0,1.0);
  RooRealVar *bkg_np_lt1 = new RooRealVar("bkg_np_lt1", "lifetime", 1.2,0.1,3.);
  RooDecay *nonprompt_tau = new RooDecay("NonPromptBkgTau", "exponential convoluted with gaussian", *tau,*bkg_np_lt1,*tau_uncert,RooDecay::SingleSided);
  // RooRealVar *bkg_np_lt2 = new RooRealVar("bkg_np_lt2", "lifetime", 0.2,0.0,3.);
  // RooDecay *np_decay2 = new RooDecay("np_decay2", "exponential convoluted with gaussian", *tau,*bkg_np_lt2,*tau_uncert,RooDecay::SingleSided);

  
  // RooAddPdf *nonprompt_tau = new RooAddPdf("NonPromptBkgTau","Non-Prompt Tau Background",RooArgSet(*np_decay1,*np_decay2),*nonprompt_ratio);

  // Mass*Tau
  RooProdPdf *nonprompt_bkg = new RooProdPdf("NonPromptBkg","Non-Prompt Background",RooArgSet(*nonprompt_mass,*nonprompt_tau));

  // nonprompt_bkg + prompt_bkg
  return new RooAddPdf("Background","Background Model",RooArgSet(*prompt_bkg,*nonprompt_bkg),*PromptFrac);
}
double get_par_val(const RooAbsCollection* pars,const char* name){
  //this sucks
  return dynamic_cast<RooRealVar*>(pars->find(name))->getVal();
}

RooAbsPdf* build_model(RooRealVar* mass, RooRealVar* tau){
  MSG("Constructing model");
  // Fraction of signal and background events
  RooRealVar *sigFrac = new RooRealVar("sigFrac","Fraction of signal events",0.5,0.,1.);

  // Lifetime uncertainty function, shared among signal and
  // background, prompt and non-prompt components
  // This also doubles as the tau signal, (ie delta fn (x) resolution)
  RooRealVar* mean_t = new RooRealVar("mean_t","Mean",0,-9.0,9.0);
  RooRealVar* sigma_t = new RooRealVar("sigma_t","Width",0.1,0.0,3.0);
  RooGaussModel* tau_uncert = new RooGaussModel("TauResolution","Tau Uncertainty",*tau,*mean_t,*sigma_t);

  RooAddPdf* Signal = build_signal(mass,tau,tau_uncert);
  RooAddPdf* Background = build_background(mass,tau,tau_uncert);
  return new RooAddPdf("model","model",RooArgList(*Signal,*Background),*sigFrac);
}
RooAbsPdf* build_psi_model(RooRealVar* mass){
  MSG("Constructing psi(2S) model");
  // Fraction of signal and background events
  RooRealVar *sigFrac = new RooRealVar("sigFrac","Fraction of signal events",0.5,0.,1.);
  // Signal mass model - Gaussian
  RooRealVar *mass_mean = new RooRealVar("psi_m_mean","Mean of #psi Mass peak",PSIMASS,PSIMASS-0.02,PSIMASS+0.02,"GeV");
  RooRealVar *mass_sigma = new RooRealVar("psi_m_sigma","Sigma of #psi peak",0.002,0.0,0.05,"GeV");
  RooGaussian *Signal = new RooGaussian("Signal", "#psi mass peak",*mass, *mass_mean, *mass_sigma);
  // RooRealVar *res_sigma = new RooRealVar("res_sigma","Resolution Sigma",0.005,0.0,0.01,"GeV");
  // RooGaussian *res_gauss = new RooGaussian("res_gauss","Resolution function",*mass,*mass_mean,*res_sigma);
  // RooRealVar *resFrac = new RooRealVar("resFrac","Fraction of resolution gaussian",0.01,0.,0.50);
  // RooAddPdf *Signal = new RooAddPdf("Signal","Signal+Gaussian",RooArgSet(*mass_gauss,*res_gauss),*resFrac);
  // ** Example ** Background mass model - 1st order polnomial
  // RooRealVar* c0 = new RooRealVar("c0","Slope", 20, 0, 1000);
  RooRealVar* c1 = new RooRealVar("c1","Slope", 0.5, -1.,5);
  RooRealVar* c2 = new RooRealVar("c2","Slope", 0.5, -1.,1);
  RooRealVar* c3 = new RooRealVar("c3","Slope", 0.5, -1.,1);

  RooChebychev *Background = new RooChebychev("Background","Background",*mass,RooArgList(*c1,*c2,*c3));
  return new RooAddPdf("model","model",RooArgList(*Signal,*Background),*sigFrac);
}
RooFitResult* Fit(RooAbsPdf* model,RooDataSet& data){
  return model->fitTo(data,RooFit::NumCPU(sysconf(_SC_NPROCESSORS_ONLN),kTRUE),RooFit::Save());
}
static void add_component(RooPlot* frame,RooAbsPdf* model,
			  const char* comp_name, 
			  const Int_t color){
  model->plotOn(frame,RooFit::Components(comp_name),
		RooFit::LineWidth(2),
		RooFit::LineColor(color),
		RooFit::LineStyle(2),
		RooFit::Name(comp_name));
}
static void add_leg_comp(TLegend* leg,RooPlot* frame, const char* comp_name){
  //char name[100];
  //snprintf(name,100,"%s%s",comp_name,key);
  RooCurve * curve = frame->getCurve(comp_name);
  leg->AddEntry(curve,comp_name,"l");
}
void print_plot(RooRealVar* var,RooDataSet* data, RooAbsPdf* model, const char* key,const char* title){
  RooPlot* frame = var->frame(200);
  TLegend* leg = new TLegend(0.7,0.85,1.0,1.0);
  leg->SetBorderSize(0);
  leg->SetFillColor(0);

  data->plotOn(frame,RooFit::Name("PlotData"));
  model->plotOn(frame,
		RooFit::LineColor(kBlue),
		RooFit::LineWidth(2),
		RooFit::NumCPU(sysconf(_SC_NPROCESSORS_ONLN),kTRUE),
		RooFit::Name("PlotModel"));

  if(std::string(key)=="mass" || std::string(key) == "psi_m"){
    add_component(frame,model,"Signal",kAzure);
    add_component(frame,model,"Background",kRed);
    add_leg_comp(leg,frame,"Signal");
    add_leg_comp(leg,frame,"Background");
  }
  if(std::string(key)=="tau"){
    add_component(frame,model,"PromptBkgTau",kRed-3);
    add_component(frame,model,"NonPromptBkgTau",kRed-1);
    add_component(frame,model,"PromptTauSig",kAzure-3);
    add_component(frame,model,"NonPromptSigTau",kAzure-1);
    add_leg_comp(leg,frame,"PromptBkgTau");
    add_leg_comp(leg,frame,"NonPromptBkgTau");
    add_leg_comp(leg,frame,"PromptTauSig");
    add_leg_comp(leg,frame,"NonPromptSigTau");
    frame->SetAxisRange(-1.1,3.0);
  }
  TCanvas canv(key,key,600,600);
  frame->Draw();
  leg->Draw();
  frame->SetTitle(title);
  char OFName[100];
  snprintf(OFName,100,"%s_fit_final.pdf",key);
  if(std::string(key)=="tau"){
    canv.SetLogy(true);
  }
  canv.SaveAs(OFName);
}
