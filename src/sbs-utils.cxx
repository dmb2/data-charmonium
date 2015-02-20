#include <vector>
#include <map>

#include "TTree.h"
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

void add_region(RooRealVar* var, const char* type, double min, double max){
  char name[400];
  snprintf(name,400,"%-3s %.4g < %s && %s < %.4g", type, min,
	   var->GetName(), var->GetName(), max);
  var->setRange(name,min,max);
}
double get_yield(RooAbsPdf* PDF, RooRealVar* var, const char* key){
  std::list<std::string> regions = var->getBinningNames();
  RooAbsReal* integral=NULL;
  double yield(0.);
  for(std::list<std::string>::const_iterator r = regions.begin();
      r!=regions.end(); ++r){
    if(r->find(key)!=std::string::npos){
      integral=PDF->createIntegral(*var,RooFit::Range(r->c_str()));
      yield += integral->getVal();
    }
  }
  return yield;
}
void print_sbs_result(TTree* tree, TH1* base_hist, const char* suffix, 
		      std::list<std::string> regions , double stsratio){
  TCanvas c1("Canvas","Canvas",600,600);
  char clone_name[100];
  TLegend& leg = *init_legend();
  std::map<std::string,std::string> leg_map;
  // THStack stack;
  snprintf(clone_name, 100, "%s_sbs",base_hist->GetName());
  // snprintf(cut_expr, 512, "");
  std::string sb_cut_expr;
  std::string sig_cut_expr;
  for(std::list<std::string>::const_iterator r = regions.begin();
      r!=regions.end(); ++r){
    if(*r == ""){
      continue;
    }
    if(r->find("SB")!=std::string::npos){
      if(sb_cut_expr.size() > 0){
	sb_cut_expr = sb_cut_expr + " && (" + r->substr(4) + ")";
      }
      else{
	sb_cut_expr = "(" + r->substr(4) + ")";
      }
    }
    if(r->find("Sig")!=std::string::npos){
      sig_cut_expr = sig_cut_expr + " && (" + r->substr(4) + ")";
    }
    else{
      sig_cut_expr = "(" + r->substr(4) + ")";
    }
  }
  MSG_DEBUG("Signal: "<<sig_cut_expr);
  MSG_DEBUG("SideBand: "<<sb_cut_expr);
  TH1* sig_hist = make_normal_hist(base_hist, tree, base_hist->GetName(),
				   sig_cut_expr.c_str(),"_sbs_sig");
  
  TH1* sb_hist  = make_normal_hist(base_hist, tree, base_hist->GetName(),
				   sb_cut_expr.c_str(),"sbs_sb");
  
  sig_hist->Add(sb_hist,-stsratio);
  sig_hist->Draw("H");
  // stack.Draw();
  leg.Draw();
  char outname[256];
  snprintf(outname,256,"%s%s",base_hist->GetName(),suffix);
  c1.SaveAs(outname);
}
void do_sbs(const char** variables, const size_t n_vars,
	    TTree* tree, RooAbsPdf* model, RooRealVar* mass,
	    const char* suffix){

  TIterator *iter = model->getComponents()->createIterator();
  RooAbsArg* var = NULL;
  RooAbsPdf* bkg = NULL;

  std::map<std::string,TH1D*> HistBook;
  init_hist_book(HistBook);


  while((var = dynamic_cast<RooAbsArg*>(iter->Next()))){
    std::string name(var->GetName());
    if(name.find("bkgModel")!=std::string::npos){
      bkg=dynamic_cast<RooAbsPdf*>(var);
    }
  }
  if(bkg == NULL){
    MSG_ERR("Could not find \"bkgModel\" in the model PDF!");
  }
  double sig_yield = get_yield(bkg, mass,"Sig");
  double sb_yield = get_yield(bkg, mass, "SB");
  // MSG_DEBUG("Calculated a Signal-to-Sideband ratio of: "<<sig_yield/sb_yield);
  for(size_t i=0; i < n_vars; i++){
    // MSG_DEBUG("Now subtracting: "<< variables[i]);
    print_sbs_result(tree,HistBook[variables[i]],suffix,mass->getBinningNames(),sig_yield/sb_yield);
  }
}
