#include "fit-utils.hh"
#include "histo-utils.hh"
#include "root-sugar.hh"

#include "TColor.h"

#include "RooAddModel.h"
#include "RooAddPdf.h"
#include "RooCBShape.h"
#include "RooChebychev.h"
#include "RooDataSet.h"
#include "RooDecay.h"
#include "RooExponential.h"
#include "RooExtendPdf.h"
#include "RooFit.h"
#include "RooFitResult.h"
#include "RooFitResult.h"
#include "RooGaussModel.h"
#include "RooGaussian.h"
#include "RooGenericPdf.h"
#include "RooPlot.h"
#include "RooPolynomial.h"
#include "RooProdPdf.h"
#include "RooProduct.h"
#include "RooRealVar.h"
#include "RooRealVar.h"

#include "TCanvas.h"
#include "TLegend.h"
RooAbsPdf* mass_signal_pdf(RooRealVar* mass, const int alpha=10,const int n =1){
  RooRealVar* mean_m = new RooRealVar("mean_m","Mean Mass",JPSIMASS,JPSIMASS-0.02,JPSIMASS+0.02);
  RooRealVar* sigma_m = new RooRealVar("sigma_m","Width of Mass",0.09543,0.0,0.5);
  RooGaussian* gauss = new RooGaussian("gauss","Mass Gaussian",*mass,*mean_m,*sigma_m);
  // 5+/- 12
  RooRealVar* cb_alpha = new RooRealVar("cb_alpha","Alpha of Crystal Ball",5);
  RooRealVar* cb_n = new RooRealVar("cb_n","N of Crystal Ball",1,0,10);
  RooRealVar* cb_sf = new RooRealVar("cb_sf","Scaling factor for CB Width",0.52762,0,1);
  RooProduct* cb_sigma = new RooProduct("cb_sigma","Crystal Ball Width",RooArgList(*sigma_m,*cb_sf));

  RooCBShape* crystal_ball = new RooCBShape("crystal_ball","Crystal Ball Mass",*mass,*mean_m,*cb_sigma,*cb_alpha,*cb_n);
  
  RooRealVar* mass_frac = new RooRealVar("mass_frac","Fraction of CB to Gauss",0.72,0,1);
  return new RooAddPdf("SignalMass","Mass Signal Model",RooArgList(*crystal_ball,*gauss),*mass_frac);
}
RooAbsPdf* signal_pdf(RooRealVar* mass, RooRealVar* tau, RooResolutionModel* tau_resolution, const int alpha=10,const int n =1){
  RooAbsPdf* signal_mass = mass_signal_pdf(mass,alpha,n);
  
  RooRealVar* lifetime = new RooRealVar("lifetime","Lifetime of J/\\psi",0.34,0,0.5);
  RooDecay* nonprompt_tau = new RooDecay("NonPromptTauSig","Non Prompt J/psi Tau",*tau,*lifetime,*tau_resolution,RooDecay::SingleSided);

  RooRealVar* prompt_frac = new RooRealVar("prompt_frac","Prompt Fraction",0.34,0,1);
  RooAddPdf* signal_tau = new RooAddPdf("TauSig","Tau Signal",RooArgList(*tau_resolution,*nonprompt_tau),*prompt_frac);
  return new RooProdPdf("Signal","Signal PDF",RooArgList(*signal_mass,*signal_tau));
}
RooAbsPdf* background_pdf(RooRealVar* mass, 
			  RooRealVar* tau, 
			  RooResolutionModel* tau_resolution,
			  const size_t n=1,
			  const bool flat_mass=false,
			  const bool flipped=false){
  RooRealVar* ncm = new RooRealVar("ncm","Non Coherent Mass Decay Constant",0.0);
  RooAbsPdf* non_coherent_mass_bkg = NULL;
  if(flat_mass){
    non_coherent_mass_bkg = new RooPolynomial("NonCoherentMassBkg","Non Coherent Mass Background",*mass);
  }
  else {
    non_coherent_mass_bkg = new RooExponential("NonCoherentMassBkg","Non Coherent Mass Background",*mass,*ncm);
  }

  RooRealVar* nclt = new RooRealVar("nclt","Non Coherent Lifetime",0.15,0,0.5);
  RooDecay* non_coherent_tau_bkg = new RooDecay("NonCoherentTauBkg","Non Coherent Tau Decay",*tau,*nclt,*tau_resolution, flipped ? RooDecay::Flipped : RooDecay::DoubleSided);
  RooProdPdf* non_coherent_bkg = new RooProdPdf("non_coherent_bkg","Non Coherent Background",RooArgList(*non_coherent_mass_bkg,*non_coherent_tau_bkg));

  RooRealVar* nplt = new RooRealVar("nplt","Non Prompt Lifetime",0.10,0,2);
  RooDecay* non_prompt_tau_bkg = new RooDecay("NonPromptTauBkg","Non Prompt Tau Decay",*tau,*nplt,*tau_resolution,RooDecay::SingleSided);
  
  RooRealVar* Pc1 = new RooRealVar("Pc1","Slope", -0.26, -0.5,0.5);
  RooRealVar* Pc2 = new RooRealVar("Pc2","Slope", 0, -0.5,0.5);
  RooRealVar* Pc3 = new RooRealVar("Pc3","Slope", 0, -0.5,0.5);
  RooPolynomial* non_prompt_mass_bkg = NULL;
  switch(n){
  case 1:
    non_prompt_mass_bkg = new RooPolynomial("NonPromptMassBkg","Linear NonPrompt Mass Background",*mass,RooArgList(*Pc1));
    break;
  case 2:
    non_prompt_mass_bkg = new RooPolynomial("NonPromptMassBkg","Linear NonPrompt Mass Background",*mass,RooArgList(*Pc1,*Pc2));
    break;
  case 3:
    non_prompt_mass_bkg = new RooPolynomial("NonPromptMassBkg","Linear NonPrompt Mass Background",*mass,RooArgList(*Pc1,*Pc2,*Pc3));
    break;
  case 0:
    non_prompt_mass_bkg = new RooPolynomial("NonPromptMassBkg","Linear NonPrompt Mass Background",*mass);
    break;
  default:
    abort();
  }
  if(non_prompt_mass_bkg==NULL){
    MSG_ERR("Mass background is undefined aborting!");
    exit(1);
  }
  RooProdPdf* non_prompt_bkg = new RooProdPdf("non_prompt_bkg","Non Prompt Background",RooArgList(*non_prompt_mass_bkg,*non_prompt_tau_bkg));

  RooPolynomial *prompt_mass_bkg = new RooPolynomial("PromptMassBkg","Flat Prompt Mass Background",*mass);
  RooProdPdf* prompt_bkg = new RooProdPdf("prompt_bkg","Prompt Background",RooArgList(*prompt_mass_bkg,*tau_resolution));

  RooRealVar* prompt_bkg_frac = new RooRealVar("prompt_bkg_frac","Prompt Background Fraction",0.65,0.0,1.0);
  RooRealVar* non_prompt_bkg_frac = new RooRealVar("non_prompt_bkg_frac","Non-prompt Background Fraction",0.29,0.0,1.0);
  return new RooAddPdf("Background","Background PDF",
		       RooArgList(*prompt_bkg,*non_prompt_bkg,*non_coherent_bkg),
		       RooArgList(*prompt_bkg_frac,*non_prompt_bkg_frac));
}
RooAbsPdf* build_model(RooRealVar* mass, RooRealVar* tau, const double n_events,
		       const int mass_n, const bool res_single,
		       const bool flat_mass,
		       const bool flipped,
		       const int cb_alpha,const int cb_n){
  MSG("Constructing model");
  // Lifetime uncertainty function, shared among signal and
  // background, prompt and non-prompt components
  // This also doubles as the tau signal, (ie delta fn (x) resolution)
  RooRealVar* mean_t1 = new RooRealVar("mean_t1","Mean",0.0);
  RooRealVar* sigma_t1 = new RooRealVar("sigma_t1","Width",0.010925,0.0,0.5);
  RooGaussModel* tau_uncert1 = new RooGaussModel("TauResolution1","Tau Uncertainty",*tau,*mean_t1,*sigma_t1);

  RooRealVar* mean_t2 = new RooRealVar("mean_t2","Mean",0.0);
  RooRealVar* sigma_t2 = new RooRealVar("sigma_t2","Width",0.032176,0.0,0.5);
  RooGaussModel* tau_uncert2 = new RooGaussModel("TauResolution2","Tau Uncertainty",*tau,*mean_t2,*sigma_t2);
  RooRealVar* res_frac = new RooRealVar("res_frac","Resolution Fraction",0.69,0.0,1.0);
  RooResolutionModel* tau_uncert=NULL;
  if(!res_single){
    tau_uncert = new RooAddModel("PromptTauSig","Double Gaussian Resolution",RooArgList(*tau_uncert1,*tau_uncert2),*res_frac);
  }
  else {
    tau_uncert = tau_uncert1;
    tau_uncert->SetName("PromptTauSig");
  }

  RooAbsPdf* Signal = signal_pdf(mass,tau,tau_uncert,cb_alpha,cb_n);
  RooAbsPdf* Background = background_pdf(mass,tau,tau_uncert,mass_n,flat_mass,flipped);
  RooRealVar* nsig=new RooRealVar("nsig","Signal Yield", 0.8*n_events,0,n_events);
  RooRealVar* nbkg= new RooRealVar("nbkg","Background Yield", 0.2*n_events,0,n_events);
  RooExtendPdf* ext_sig = new RooExtendPdf("ext_sig","Extended Signal PDF",*Signal,*nsig);
  RooExtendPdf* ext_bkg = new RooExtendPdf("ext_bkg","Extended Background PDF",*Background,*nbkg);
  
  return new RooAddPdf("model","model",RooArgList(*ext_sig,*ext_bkg));
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
  return model->fitTo(data,RooFit::NumCPU(sysconf(_SC_NPROCESSORS_ONLN),kTRUE),RooFit::Save(),RooFit::Extended());
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
  std::map<std::string,std::string> pretty_names;
  pretty_names["PromptTauSig"]="Prompt Signal";
  pretty_names["NonPromptTauSig"]="Non-prompt";
  pretty_names["PlotModel"]="Total";
  pretty_names["Signal"]="Signal";
  pretty_names["Background"]="Background";
  leg->AddEntry(curve,pretty_names[comp_name].c_str(),"l");
}
void print_plot(RooRealVar* var,RooDataSet* data, RooAbsPdf* model, 
		const char* key,const char* title, 
		const double lumi, const char* syst_var){
  RooPlot* frame = var->frame(200);
  TLegend* leg = init_legend();//new TLegend(0.7,0.85,1.0,1.0);
  leg->SetBorderSize(0);
  leg->SetFillColor(0);

  data->plotOn(frame,RooFit::Name("PlotData"));
  model->plotOn(frame,
		RooFit::LineColor(kRed),
		RooFit::LineWidth(2),
		RooFit::NumCPU(sysconf(_SC_NPROCESSORS_ONLN),kTRUE),
		RooFit::Name("PlotModel"));
  add_leg_comp(leg,frame,"PlotModel");
  if(std::string(key)=="mass" || std::string(key) == "psi_m"){
    add_component(frame,model,"Signal",kRed);
    add_component(frame,model,"Background",kBlue);
    add_leg_comp(leg,frame,"Signal");
    add_leg_comp(leg,frame,"Background");
    frame->SetMaximum(1.2*frame->GetMaximum());
  }
  if(std::string(key)=="tau"){
    add_component(frame,model,"Background",TColor::GetColor(27,158,119));
    // add_component(frame,model,"NonPromptTauBkg",kRed-1);
    add_component(frame,model,"PromptTauSig",TColor::GetColor(216,95,2));
    add_component(frame,model,"NonPromptTauSig",TColor::GetColor(117,112,179));
    add_leg_comp(leg,frame,"Background");
    // add_leg_comp(leg,frame,"NonPromptTauBkg");
    add_leg_comp(leg,frame,"PromptTauSig");
    add_leg_comp(leg,frame,"NonPromptTauSig");
    frame->SetMaximum(5*frame->GetMaximum());
    frame->SetAxisRange(-0.5,1.0);
  }
  TCanvas canv(key,key,600,600);
  frame->Draw();
  leg->Draw();
  frame->SetTitle(title);
  char OFName[100];
  if(syst_var!=NULL){
    snprintf(OFName,100,"%s_%s_fit_final.pdf",key,syst_var);
  }
  else{
    snprintf(OFName,100,"%s_fit_final.pdf",key);
  }
  if(std::string(key)=="tau"){
    canv.SetLogy(true);
  }
  add_atlas_badge(canv,0.2,0.9,lumi);
  canv.SaveAs(OFName);
  // snprintf(OFName,100,"%s_fit_final.root",key);
  // canv.SaveAs(OFName);
}
