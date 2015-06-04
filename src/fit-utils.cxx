#include "fit-utils.hh"
#include "histo-utils.hh"
#include "root-sugar.hh"

#include "RooFit.h"
#include "RooDataSet.h"
#include "RooRealVar.h"
#include "RooGaussian.h"
#include "RooProduct.h"
#include "RooCBShape.h"
#include "RooGenericPdf.h"
#include "RooChebychev.h"
#include "RooPolynomial.h"
#include "RooGaussModel.h"
#include "RooAddModel.h"
#include "RooAddPdf.h"
#include "RooPlot.h"
#include "RooFitResult.h"
#include "RooProdPdf.h"
#include "RooDecay.h"
#include "RooRealVar.h"
#include "RooFitResult.h"

#include "TCanvas.h"
#include "TLegend.h"
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
  RooRealVar* mean_t1 = new RooRealVar("mean_t1","Mean",0.0);
  RooRealVar* sigma_t1 = new RooRealVar("sigma_t1","Width",0.1,0.0,3.0);
  RooGaussModel* tau_uncert1 = new RooGaussModel("TauResolution1","Tau Uncertainty",*tau,*mean_t1,*sigma_t1);

  RooRealVar* mean_t2 = new RooRealVar("mean_t2","Mean",0.0);
  RooRealVar* sigma_t2 = new RooRealVar("sigma_t2","Width",0.1,0.0,3.0);
  RooGaussModel* tau_uncert2 = new RooGaussModel("TauResolution2","Tau Uncertainty",*tau,*mean_t2,*sigma_t2);
  RooRealVar* res_frac = new RooRealVar("res_frac","Resolution Fraction",0.5,0.0,1.0);
  RooAddModel* tau_uncert = new RooAddModel("TauResolution","Double Gaussian Resolution",RooArgList(*tau_uncert1,*tau_uncert2),*res_frac);

  // RooAddPdf* Signal = build_signal(mass,tau,tau_uncert);
  // RooAddPdf* Background = build_background(mass,tau,tau_uncert);
  // return new RooAddPdf("model","model",RooArgList(*Signal,*Background),*sigFrac);
  return NULL;
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
void print_plot(RooRealVar* var,RooDataSet* data, RooAbsPdf* model, 
		const char* key,const char* title, 
		const double lumi){
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
  }
  if(std::string(key)=="tau"){
    add_component(frame,model,"Background",kBlue);
    // add_component(frame,model,"NonPromptBkgTau",kRed-1);
    add_component(frame,model,"PromptTauSig",kMagenta);
    add_component(frame,model,"NonPromptSigTau",kAzure);
    add_leg_comp(leg,frame,"Background");
    // add_leg_comp(leg,frame,"NonPromptBkgTau");
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
  add_atlas_badge(canv,0.2,0.9,lumi,INTERNAL);
  canv.SaveAs(OFName);
}
