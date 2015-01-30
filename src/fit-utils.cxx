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
  RooRealVar *sigPromptFrac = new RooRealVar("sigPromptFrac","Fraction of signal events",0.5,0.,1.);
  RooRealVar *bkgPromptFrac = new RooRealVar("bkgPromptFrac","Fraction of signal events",0.5,0.,1.);
	
  // Signal mass model - Gaussian
  RooRealVar *mMean = new RooRealVar("mMean","mean of gaussian",PDGMASS,PDGMASS-0.02,PDGMASS+0.02);
  RooRealVar *mSigma = new RooRealVar("mSigma","Scale Factor 1",0.005,0.0,0.05);
  RooGaussian *mSig = new RooGaussian("PromptMassSig","signal p.d.f.",*mass,*mMean,*mSigma); 

  RooGaussian *npmSig = new RooGaussian("NonPromptMassSig","signal p.d.f.",*mass,*mMean,*mSigma); 

  /*
  char gen_pdf_str[100];
  snprintf(gen_pdf_str,100,"1+(5*(@0-%.4g))*@1+((5*(@0-%.4g))*(5*(@0-%.4g)))*@2",PDGMASS,PDGMASS,PDGMASS);
  RooRealVar *c0 = new RooRealVar("c0","Bkg coeff c0",0.3,  0.0, 1.0) ;
  RooRealVar *c1 = new RooRealVar("c1","Bkg coeff c1",0.,  -3.0, 3.0) ;
  RooGenericPdf *mBkg = new RooGenericPdf("PromptMassBkg","Background",gen_pdf_str,RooArgList(*mass,*c0,*c1));
	
  RooRealVar *npc0 = new RooRealVar("npc0","Bkg coeff c0",0.3,  0.0, 1.0) ;
  RooRealVar *npc1 = new RooRealVar("npc1","Bkg coeff c1",0.,  -3.0, 3.0) ;
  RooGenericPdf *npmBkg = new RooGenericPdf("NonPromptMassBkg","Background",gen_pdf_str,RooArgList(*mass,*npc0,*npc1));
  */	
  // ** Example **  Background mass model - 1st order polnomial
  RooRealVar* c= new RooRealVar("c","Slope", -0.00015, -0.01,0.01);
  RooPolynomial* mBkg = new RooPolynomial("PromptMassBkg","background",*mass,RooArgList(*c));
  
  RooRealVar* npc= new RooRealVar("npc","Slope", -0.00015, -0.01,0.01);
  RooPolynomial *npmBkg = new RooPolynomial("NonPromptMassBkg","Background",*mass,RooArgList(*npc));
  // Lifetime uncertainty model
  RooRealVar *tMean = new RooRealVar("tMean","mean of gaussian",0,-9.,9.);
  RooRealVar *tSigma = new RooRealVar("tSigma","Scale Factor 1",0.1,0.005,3.0);
  RooGaussModel *tGauss = new RooGaussModel("tGauss2","Gaussian",*tau,*tMean,*tSigma);

  // Signal lifetime model
  RooRealVar *fittedTau = new RooRealVar("fittedTau", "lifetime", 1.53,1.0,2.0);
  RooDecay *tSig = new RooDecay("PromptTauSig", "exponential convoluted with gaussian", *tau,*fittedTau,*tGauss,RooDecay::SingleSided);

  // Background lifetime model
  RooRealVar *bkgTau1 = new RooRealVar("bkgTau1", "lifetime", 1.2,0.1,3.);
  RooDecay *bkgDecay1 = new RooDecay("bkgDecay1", "exponential convoluted with gaussian", *tau,*bkgTau1,*tGauss,RooDecay::SingleSided);
  RooRealVar *bkgTau2 = new RooRealVar("bkgTau2", "lifetime", 0.2,0.1,3.);
  RooDecay *bkgDecay2 = new RooDecay("bkgDecay2", "exponential convoluted with gaussian", *tau,*bkgTau2,*tGauss,RooDecay::SingleSided);
  RooRealVar *nonPromptRatio = new RooRealVar("nonPromptRatio", "Ratio between background decays", 0.25,0.,1.);
  RooRealVar *promptRatio = new RooRealVar("promptRatio", "Ratio between background decays", 0.8,0.,1.);
  RooAddPdf *tNonPrompt = new RooAddPdf("NonPromptTauBkg", "background model", RooArgList(*bkgDecay1,*bkgDecay2), *nonPromptRatio);

  RooGaussian *promptGauss = new RooGaussian("prompt1","prompt particles",*tau,*tMean,*tSigma);
  RooRealVar *bkgTau3 = new RooRealVar("bkgTau3", "lifetime", 0.15,0.1,3.);
  RooDecay *bkgDecay3 = new RooDecay("bkgDecay3", "exponential convoluted with gaussian", *tau,*bkgTau3,*tGauss,RooDecay::DoubleSided);
  RooAddPdf *tPrompt = new RooAddPdf("PromptTauBkg", "background model", RooArgList(*promptGauss,*bkgDecay3), *promptRatio);

  //signal prompt - mSig * tGauss
  RooProdPdf *sigPrompt = new RooProdPdf("sigPrompt", "signal Prompt Model", RooArgSet(*mSig,*tGauss));
  
  //background prompt - mBkg * tPrompt
  RooProdPdf *bkgPrompt = new RooProdPdf("bkgPrompt", "background Prompt Model", RooArgSet(*mBkg,*tPrompt));
  
  //signal non prompt - mSig * tSig
  RooProdPdf *sigNonPrompt = new RooProdPdf("sigNonPrompt", "signal NonPrompt Model", RooArgSet(*npmSig,*tSig));
  
  //background non prompt - npmBkg * tNonPrompt
  RooProdPdf *bkgNonPrompt = new RooProdPdf("bkgNonPrompt", "background NonPrompt Model", RooArgSet(*npmBkg,*tNonPrompt));
	
  //Complete model - Makes a 2D fit
  RooAddPdf *sigModel = new RooAddPdf("sigModel", "signal Model", RooArgSet(*sigPrompt,*sigNonPrompt),*sigPromptFrac);
  RooAddPdf *bkgModel = new RooAddPdf("bkgModel", "background Model", RooArgSet(*bkgPrompt,*bkgNonPrompt),*bkgPromptFrac);

  return new RooAddPdf("model","model",RooArgList(*sigModel,*bkgModel),*sigFrac);
}
RooFitResult* Fit(RooAbsPdf* model,RooDataSet& data){
  return model->fitTo(data,RooFit::NumCPU(4,kTRUE),RooFit::Save());
}
static void add_component(RooPlot* frame,RooAbsPdf* model,
			  const char* key, const char* comp_name, const Int_t color){
  char components[100];
  char name[100];
  //eg PromptMassSig,PromptMassBkg
  snprintf(components,100,"%s%sSig,%s%sBkg",comp_name,key,comp_name,key);
  //eg PromptMass
  snprintf(name,100,"%s%s",comp_name,key);
  model->plotOn(frame,RooFit::Components(components),
		RooFit::LineWidth(2),
		RooFit::LineColor(color),
		RooFit::LineStyle(2),
		RooFit::Name(name));
}
static void add_leg_comp(TLegend* leg,RooPlot* frame, const char* key, const char* comp_name){
  char name[100];
  snprintf(name,100,"%s%s",comp_name,key);
  RooCurve * curve = frame->getCurve(name);
  leg->AddEntry(curve,comp_name,"l");
}
static void print_plot(RooRealVar* var,RooDataSet* data, RooAbsPdf* model, const char* key,const char* title){
  RooPlot* frame = var->frame();
  data->plotOn(frame,RooFit::Name("PlotData"));
  model->plotOn(frame,
		RooFit::LineColor(kBlue),
		RooFit::LineWidth(2),
		RooFit::NumCPU(4,kTRUE),
		RooFit::Name("PlotModel"));
  add_component(frame,model,key,"Prompt", kAzure);
  add_component(frame,model,key,"NonPrompt",kViolet);
  TLegend* leg = new TLegend(0.7,0.85,1.0,1.0);
  leg->SetBorderSize(0);
  leg->SetFillColor(0);
  add_leg_comp(leg,frame,key,"Prompt");
  add_leg_comp(leg,frame,key,"NonPrompt");
  TCanvas canv(key,key,600,600);
  frame->Draw();
  leg->Draw();
  frame->SetTitle(title);
  char OFName[100];
  snprintf(OFName,100,"%s_fit_final.pdf",key);
  canv.SetLogy(std::string(key)=="Tau");
  canv.SaveAs(OFName);
}

void print_fit_results(RooAbsPdf* model,RooDataSet* data, RooRealVar* mass, RooRealVar *tau) {
  print_plot(mass,data,model,"Mass",";J/#psi Mass [GeV]");
  print_plot(tau,data,model,"Tau",";J/#psi Proper Decay Time [ps]");
}
