#pragma once
#include <string>
#include <list>
#include <cstddef>
#include "math.hh"

class TTree;
class RooRealVar;
class RooAbsPdf;

void do_sbs(const char** variables, const size_t n_vars,
	    TTree* tree, RooAbsPdf* model, 
	    RooRealVar* mass, RooRealVar* tau,
	    RooRealVar* psi_m, num_err psi_stsR, 
	    const char* suffix);
void add_region(RooRealVar* var, const char* type, double min, double max);
std::string make_cut_expr(const std::list<std::string>& regions, 
			  const std::string& key);
RooAbsPdf* find_component(RooAbsPdf* PDF,const char* name);
num_err get_yield(RooAbsPdf* PDF, RooRealVar* var, const char* key);
