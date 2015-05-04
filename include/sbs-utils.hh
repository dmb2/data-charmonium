#pragma once
#include <string>
#include <list>
#include <map>
#include <cstddef>
#include "math.hh"

class TTree;
class RooRealVar;
class RooAbsPdf;
class TH1;
typedef struct{
  std::list<std::string> regions;
  num_err sts_ratio;
} sb_info;

TH1* print_sbs_stack(TTree* tree, TH1* base_hist, const char* suffix,
		     std::map<std::string,sb_info> sep_var_info,
		     const double lumi);
void print_pythia_stack(TH1* base_hist, TH1* signal,
			const double lumi,const char* cut_expr,
			const char* suffix);
void add_region(RooRealVar* var, const char* type, double min, double max);
std::string make_cut_expr(const std::list<std::string>& regions, 
			  const std::string& key);
RooAbsPdf* find_component(RooAbsPdf* PDF,const char* name);
num_err get_yield(RooAbsPdf* PDF, RooRealVar* var, 
		  const char* key, const double* covmat);
