#include <iostream>
#include "root-sugar.hh"
#include "fit-utils.hh"

int main(const int argc, const char* argv[]){
  
  RooAddPdf* model = build_model();
  RooArgSet* result = Fit(model);
  print_fit_results(model);
  
  return 0;
}
