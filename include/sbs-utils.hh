#pragma once
#include <cstddef>
class TTree;
class RooRealVar;
class RooAbsPdf;

void do_sbs(const char** variables, const size_t n_vars,
	    TTree* tree, RooAbsPdf* model, RooRealVar* mass, 
	    RooRealVar* tau, const char* suffix);
void add_region(RooRealVar* var, const char* type, double min, double max);

