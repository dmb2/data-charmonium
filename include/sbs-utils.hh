#pragma once
#include <string>
#include <list>
#include <map>

#include <cstddef>
#include "math.hh"

class TTree;
class RooRealVar;
class RooAbsPdf;
class RooAbsCollection;
class RooWorkspace;
class TH1;

typedef struct{
  std::list<std::string> regions;
  num_err sts_ratio;
} sb_info;

void print_bkg_splot(TTree* tree, TH1* hist,const char* suffix, const double lumi,RooWorkspace* wkspc);
std::pair<TH1*,TH1*> make_splot(TTree* tree, TH1* base_hist, RooWorkspace* wkspc);
void print_splot_stack(TTree* tree,TH1* base_hist,TH1* sig_final,TH1* bkg_final,const char* signal_cut_expr, const double lumi, const char* suffix);
TH1* print_sbs_stack(TTree* tree, TH1* base_hist, const char* suffix,
		     std::map<std::string,sb_info> sep_var_info,
		     const double lumi);
void print_pythia_stack(TH1* base_hist, TH1* signal,
			const double lumi,const char* cut_expr,
			const char* suffix);
double get_par_val(const RooAbsCollection* pars,const char* name);
void add_region(RooRealVar* var, const char* type, double min, double max);
std::string make_cut_expr(const std::list<std::string>& regions, 
			  const std::string& key);
RooAbsPdf* find_component(RooAbsPdf* PDF,const char* name);
num_err get_yield(RooAbsPdf* PDF, RooRealVar* var, 
		  const char* key, const double* covmat);
