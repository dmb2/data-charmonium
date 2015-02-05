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


/*
std::string snip_str(const std::string& base,const std::string& to_remove){
  //removes the substring 'to_remove' from the base string
  size_t idx=base.find(to_remove);
  if(idx!=std::string::npos){
    return base.substr(0,idx)+base.substr(idx+to_remove.size());
  }
  return base;
}
*/
void set_integral_limits(RooRealVar* var, const char* range_name,
			 const char* key){
  var->setRange(range_name,
		var->getBinning(key).binLow(0),
		var->getBinning(key).binHigh(0));
}

std::pair<std::string,double> do_integral(RooAbsPdf* PDF,
					  RooRealVar* var1,
					  RooRealVar* var2,
					  const char* v1_range,
					  const char* v2_range){
  double result(0.);
  char range_name[256];
  snprintf(range_name,256,"(%s) && (%s)",v1_range+4,v2_range+4);
  set_integral_limits(var1,range_name,v1_range);
  set_integral_limits(var2,range_name,v2_range);
  result = PDF->createIntegral(RooArgSet(*var1,*var2),
			       RooArgSet(*var1,*var2),
			       range_name)->getVal();
  return std::pair<std::string,double>(range_name,result);
}

double get_signal_yield(RooAbsPdf* PDF, RooRealVar* var1, RooRealVar* var2){
  // Look familiar?
  std::list<std::string> range1 = var1->getBinningNames();
  std::list<std::string> range2 = var2->getBinningNames();
  for(std::list<std::string>::const_iterator ri = range1.begin();
      ri!=range1.end(); ++ri){
    for(std::list<std::string>::const_iterator rj = range2.begin();
    	rj != range2.end(); ++rj){
      if(ri->find("Sig")!=std::string::npos &&
	 rj->find("Sig")!=std::string::npos){
      	// MSG_DEBUG(*ri << " "<<*rj);
      	return do_integral(PDF,var1,var2,ri->c_str(),rj->c_str()).second;
      }
    }
  }

  return 0;
}
void cache_integrals(std::map<std::string,double>& region_integrals,
		     RooAbsPdf *PDF, RooRealVar* var1,RooRealVar* var2){
  std::list<std::string> range1 = var1->getBinningNames();
  std::list<std::string> range2 = var2->getBinningNames();
  for(std::list<std::string>::const_iterator ri= range1.begin();
      ri!=range1.end(); ++ri){
    if(*ri==""){
      continue;
    }
    for(std::list<std::string>::const_iterator rj=range2.begin();
	rj!=range2.end(); ++rj){
      if(*rj==""){
      	continue;
      }
      if(ri->find("Sig")==std::string::npos ||
	 rj->find("Sig")==std::string::npos){
      	region_integrals.insert(do_integral(PDF,var1,var2,
					    ri->c_str(),
					    rj->c_str()));
      }
    }
  }
}
void add_region(RooRealVar* var, const char* type, double min, double max){
  char name[400];
  snprintf(name,400,"%-3s %.4g < %s && %s < %.4g", type, min,
	   var->GetName(), var->GetName(), max);
  var->setRange(name,min,max);
}
void print_sbs_result(TTree* tree, const std::map<std::string,double>& ratios,
		      TH1* base_hist, const char* suffix){
  TCanvas c1("Canvas","Canvas",600,600);
  char cut_expr[512];
  char clone_name[100];
  TLegend& leg = *init_legend();
  // std::map<std::string,std::string> leg_map;
  THStack stack;
  TH1* hist=NULL;
  char region_idx='A';
  char leg_name[2];
  std::vector<int> colors;
  colors.reserve(5);
  colors.push_back(TColor::GetColor(27,158,119));
  colors.push_back(TColor::GetColor(217,95,2));
  colors.push_back(TColor::GetColor(117,112,179));
  colors.push_back(TColor::GetColor(231,41,138));
  colors.push_back(TColor::GetColor(102,166,30));
  for(std::map<std::string,double>::const_iterator it=ratios.begin();
      it!=ratios.end(); ++it){
    snprintf(cut_expr,512,"%.4g*(%s)",it->second,it->first.c_str());
    snprintf(clone_name,100,"%s_%c_sbs",base_hist->GetName(),region_idx);
    // MSG_DEBUG(clone_name);
    hist = (TH1*)base_hist->Clone(clone_name);
    MSG_DEBUG(cut_expr);
    draw_histo(tree,base_hist->GetName(),hist->GetName(),cut_expr);
    stack.Add(hist);
    hist->SetLineColor(colors[region_idx-'A']);
    hist->SetFillColor(colors[region_idx-'A']);
    snprintf(leg_name,2,"%c",region_idx);
    leg.AddEntry(hist,leg_name,"f");
    // MSG_DEBUG(hist->Integral())
    region_idx++;
  }
  stack.Draw();
  leg.Draw();
  char outname[256];
  snprintf(outname,256,"%s%s",base_hist->GetName(),suffix);
  c1.SaveAs(outname);
}
void do_sbs(const char** variables, const size_t n_vars,
	    TTree* tree, RooAbsPdf* model, RooRealVar* mass,
	    RooRealVar* tau, const char* suffix){

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
  std::map<std::string,double> region_ratios;
  cache_integrals(region_ratios,model,mass,tau);
  double signal_yield = get_signal_yield(model,mass,tau);
  for(std::map<std::string,double>::iterator it=region_ratios.begin();
      it!=region_ratios.end(); ++it){
    // MSG_DEBUG("Before: "<<it->second);
    it->second=signal_yield/it->second;
    // MSG_DEBUG("After: "<<it->second);
  }
  // MSG_DEBUG(signal_yield);
  // MSG_DEBUG("Calculated a Signal-to-Sideband ratio of: "<<sig_yield/sb_yield);
  for(size_t i=0; i < n_vars; i++){
    // MSG_DEBUG("Now subtracting: "<< variables[i]);
    print_sbs_result(tree,region_ratios,HistBook[variables[i]],suffix);
  }
}
