#include "sbs-utils.hh"
#include "TTree.h"
#include "RooAbsPdf.h"
#include "RooRealVar.h"
#include "RooAbsReal.h"
#include "RooAbsArg.h"
#include "RooArgSet.h"
#include "root-sugar.hh"
#include <vector>

double get_yield(RooAbsPdf *PDF,RooRealVar* var,const char* key){
  double yield=0;
  RooAbsReal* integral = NULL;
  std::list<std::string> range_names = var->getBinningNames();
  for(std::list<std::string>::const_iterator name = range_names.begin(); 
      name != range_names.end(); ++name){
    if(name->find(key)!=std::string::npos){
      MSG_DEBUG("Integrating region: "<<*name);
      integral = PDF->createIntegral(*var,RooFit::Range(name->c_str()));
      yield+=integral->getVal();
    }
  }
  return yield;
}

void add_region(RooRealVar* var, const char* key, double min, double max){
  char name[100];
  snprintf(name,100,"%s[%.3g,%.3g]",key,min,max);
  var->setRange(name,min,max);
}

void do_sbs(const char** variables, const size_t n_vars,
	    TTree* tree, RooAbsPdf* model, RooRealVar* mass, 
	    RooRealVar* tau, const char* suffix){
  

  TIterator *iter = model->getComponents()->createIterator();
  RooAbsArg* var = NULL;
  RooAbsPdf* bkg = NULL;
  while((var = dynamic_cast<RooAbsArg*>(iter->Next()))){
    std::string name(var->GetName());
    if(name.find("bkgModel")!=std::string::npos){
      bkg=dynamic_cast<RooAbsPdf*>(var);
    }
  }
  if(bkg == NULL){
    MSG_ERR("Could not find \"bkgModel\" in the model PDF!");
  }
  double sb_yield = get_yield(bkg,mass,"SB");
  double sig_yield = get_yield(bkg,mass,"Sig");
  MSG_DEBUG("Calculated a Signal-to-Sideband ratio of: "<<sig_yield/sb_yield);
  for(size_t i=0; i < n_vars; i++){
    MSG_DEBUG("Now subtracting: "<< variables[i]);
  }
}
