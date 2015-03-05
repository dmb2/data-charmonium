#include "fit-utils.hh"
#include "root-sugar.hh"

#include "RooFit.h"
#include "RooDataSet.h"
#include "RooRealVar.h"
#include "RooGaussian.h"
#include "RooGenericPdf.h"
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

RooAbsPdf* build_model(RooRealVar* mass, RooRealVar* tau){
  MSG("Constructing model");
  // Fraction of signal and background events
  RooRealVar *sigFrac = new RooRealVar("sigFrac","Fraction of signal events",0.5,0.,1.);
	
  // Signal mass model - Gaussian
  RooRealVar *mass_mean = new RooRealVar("mass_mean","Mean of J/Psi Mass peak",PDGMASS,PDGMASS-0.02,PDGMASS+0.02,"GeV");
  RooRealVar *mass_sigma = new RooRealVar("mass_sigma","Sigma of J/Psi peak",0.005,0.0,0.05,"GeV");
  RooGaussian *mass_gauss = new RooGaussian("mass_gauss", "J/Psi mass_gauss Peak",*mass, *mass_mean, *mass_sigma);

  RooRealVar *res_sigma = new RooRealVar("res_sigma","Resolution Sigma",0.050,0.0,0.1,"GeV");
  RooGaussian *res_gauss = new RooGaussian("res_gauss","Resolution function",*mass,*mass_mean,*res_sigma);
  RooRealVar *resFrac = new RooRealVar("resFrac","Fraction of resolution gaussian",0.01,0.,0.50);
  RooAddPdf *Signal = new RooAddPdf("Signal","Signal+Gaussian",RooArgSet(*mass_gauss,*res_gauss),*resFrac);

  // ** Example **  Background mass model - 1st order polnomial
  // RooRealVar* c0 = new RooRealVar("c0","Intercept", 20, 0, 40);
  RooRealVar* c1 = new RooRealVar("c1","Slope", -0.27, -0.5,0.5);
  RooPolynomial *Background = new RooPolynomial("Background","Background",*mass,RooArgList(*c1));
  
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
static void print_plot(RooRealVar* var,RooDataSet* data, RooAbsPdf* model, const char* key,const char* title){
  RooPlot* frame = var->frame();
  TLegend* leg = new TLegend(0.7,0.85,1.0,1.0);
  leg->SetBorderSize(0);
  leg->SetFillColor(0);

  data->plotOn(frame,RooFit::Name("PlotData"));
  model->plotOn(frame,
		RooFit::LineColor(kBlue),
		RooFit::LineWidth(2),
		RooFit::NumCPU(sysconf(_SC_NPROCESSORS_ONLN),kTRUE),
		RooFit::Name("PlotModel"));

  add_component(frame,model,"Signal",kAzure);
  add_component(frame,model,"Background",kRed);
  add_leg_comp(leg,frame,"Signal");
  add_leg_comp(leg,frame,"Background");

  TCanvas canv(key,key,600,600);
  frame->Draw();
  leg->Draw();
  frame->SetTitle(title);
  char OFName[100];
  snprintf(OFName,100,"%s_fit_final.pdf",key);
  canv.SaveAs(OFName);
}

void print_fit_results(RooAbsPdf* model,RooDataSet* data, RooRealVar* mass, RooRealVar *tau) {
  print_plot(mass,data,model,"mass",";J/#psi Mass [GeV]");
  // print_plot(tau,data,model,"Tau",";J/#psi Proper Decay Time [ps]");
}
