#include "fit-utils.hh"
#include "root-sugar.hh"

#include "RooFit.h"
#include "RooDataSet.h"
#include "RooRealVar.h"
#include "RooGaussian.h"
#include "RooGenericPdf.h"
#include "RooGaussModel.h"
#include "RooAddPdf.h"
#include "RooPlot.h"
#include "RooFitResult.h"
#include "RooProdPdf.h"
#include "RooDecay.h"
#include "RooRealVar.h"
#include "RooFitResult.h"


RooAbsPdf* build_model(RooRealVar* mass, RooRealVar* tau){
  MSG("Constructing model");
  
  // Fraction of signal and background events
  RooRealVar sigFrac("sigFrac","Fraction of signal events",0.5,0.,1.);
  RooRealVar sigPromptFrac("sigPromptFrac","Fraction of signal events",0.5,0.,1.);
  RooRealVar bkgPromptFrac("bkgPromptFrac","Fraction of signal events",0.5,0.,1.);
	
  // Signal mass model - Gaussian
  RooRealVar mMean("mMean","mean of gaussian",PDGMASS,PDGMASS-0.02,PDGMASS+0.02);
  RooRealVar mSigma("mSigma","Scale Factor 1",0.005,0.0,0.05);
  RooGaussian mSig("mSig","signal p.d.f.",*mass,mMean,mSigma); 

  RooGaussian npmSig("npmSig","signal p.d.f.",*mass,mMean,mSigma); 

  //Background mass model prompt- 2nd order polynomial (if using this model change 3.7 to approximation of particle mass)
  RooRealVar c0("c0","Bkg coeff c0",0.3,  0.0, 1.0) ;
  RooRealVar c1("c1","Bkg coeff c1",0.,  -3.0, 3.0) ;
  RooGenericPdf mBkg("mBkg","Background","1+((@0-3.7)/0.2)*@1+(((@0-3.7)/0.2)*((@0-3.7)/0.2))*@2",RooArgList(*mass,c0,c1));
	
  //Background mass model non prompt - 2nd order polynomial (if using this model change 3.7 to approximation of particle mass)
  RooRealVar npc0("npc0","Bkg coeff c0",0.3,  0.0, 1.0) ;
  RooRealVar npc1("npc1","Bkg coeff c1",0.,  -3.0, 3.0) ;
  RooGenericPdf npmBkg("npmBkg","Background","1+((@0-3.7)/0.2)*@1+(((@0-3.7)/0.2)*((@0-3.7)/0.2))*@2",RooArgList(*mass,npc0,npc1));
	
  // ** Example **  Background mass model - 1st order polnomial
  //RooRealVar c("c","Slope", -0.00015, -0.01,0.01);
  //RooPolynomial mBkg("mBkg","background",mass,RooArgList(c));

  // Lifetime uncertainty model
  RooRealVar tMean("tMean","mean of gaussian",0,-9.,9.);
  RooRealVar tSigma("tSigma","Scale Factor 1",0.1,0.005,3.0);
  RooGaussModel tGauss("tGauss2","Gaussian",*tau,tMean,tSigma);

  // Signal lifetime model
  RooRealVar fittedTau("fittedTau", "lifetime", 1.53,1.0,2.0);
  RooDecay tSig("tSig", "exponential convoluted with gaussian", *tau,fittedTau,tGauss,RooDecay::SingleSided);

  // Background lifetime model
  RooRealVar bkgTau1("bkgTau1", "lifetime", 1.2,0.1,3.);
  RooDecay bkgDecay1("bkgDecay1", "exponential convoluted with gaussian", *tau,bkgTau1,tGauss,RooDecay::SingleSided);
  RooRealVar bkgTau2("bkgTau2", "lifetime", 0.2,0.1,3.);
  RooDecay bkgDecay2("bkgDecay2", "exponential convoluted with gaussian", *tau,bkgTau2,tGauss,RooDecay::SingleSided);
  RooRealVar nonPromptRatio("nonPromptRatio", "Ratio between background decays", 0.25,0.,1.);
  RooRealVar promptRatio("promptRatio", "Ratio between background decays", 0.8,0.,1.);
  RooAddPdf tNonPrompt("tNonPrompt", "background model", RooArgList(bkgDecay1,bkgDecay2), nonPromptRatio);

  RooGaussian promptGauss("prompt1","prompt particles",*tau,tMean,tSigma);
  RooRealVar bkgTau3("bkgTau3", "lifetime", 0.15,0.1,3.);
  RooDecay bkgDecay3("bkgDecay3", "exponential convoluted with gaussian", *tau,bkgTau3,tGauss,RooDecay::DoubleSided);
  RooAddPdf tPrompt("tPrompt", "background model", RooArgList(promptGauss,bkgDecay3), promptRatio);

  //signal prompt - mSig * tGauss
  RooProdPdf sigPrompt("sigPrompt", "signal Prompt Model", RooArgSet(mSig,tGauss));
  
  //background prompt - mBkg * tPrompt
  RooProdPdf bkgPrompt("bkgPrompt", "background Prompt Model", RooArgSet(mBkg,tPrompt));
  
  //signal non prompt - mSig * tSig
  RooProdPdf sigNonPrompt("sigNonPrompt", "signal NonPrompt Model", RooArgSet(npmSig,tSig));
  
  //background non prompt - npmBkg * tNonPrompt
  RooProdPdf bkgNonPrompt("bkgNonPrompt", "background NonPrompt Model", RooArgSet(npmBkg,tNonPrompt));
	
  //Complete model - Makes a 2D fit
  RooAddPdf sigModel("sigModel", "signal Model", RooArgSet(sigPrompt,sigNonPrompt),sigPromptFrac);
  RooAddPdf bkgModel("bkgModel", "background Model", RooArgSet(bkgPrompt,bkgNonPrompt),bkgPromptFrac);
  return new RooAddPdf("model","model",RooArgList(sigModel,bkgModel),sigFrac);
}
RooFitResult* Fit(RooAbsPdf* model,RooDataSet& data){
  return model->fitTo(data,RooFit::NumCPU(4,kTRUE),RooFit::Save());
}
void print_fit_results(RooAbsPdf* model){
  MSG_DEBUG("Printing fit results!");
}
