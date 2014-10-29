#include "Cut.hh"
#include <cstdio>

void print_cut_summary(std::string CutName, cut<int> Cut){
  printf("| %-8s | %8d | %6d | \n",CutName.c_str(),Cut.cut_value(),Cut.count());
}
void print_cut_summary(std::string CutName, cut<double> Cut){
  printf("| %-8s | %8.2g | %6d | \n",CutName.c_str(),Cut.cut_value(),Cut.count());
}
void print_cut_table(real_cuts& CutDefReals,category_cuts& CutDefCats,
		     const char* CutNames[],size_t nCuts){
  real_cuts::iterator rCutHandle;
  category_cuts::iterator cCutHandle;
  printf("|----------+----------+--------|\n");
  printf("| Cut Name | Cut Val  |  Count |\n");
  printf("|----------+----------+--------|\n");
  for(size_t i=0; i < nCuts; i++){
    cCutHandle=CutDefCats.find(CutNames[i]);
    if(cCutHandle!=CutDefCats.end()){
      print_cut_summary(cCutHandle->first, cCutHandle->second);
    }
    else{
      rCutHandle=CutDefReals.find(CutNames[i]);
      if(rCutHandle!=CutDefReals.end()){
	print_cut_summary(rCutHandle->first,rCutHandle->second);
      }
    }
  }
  printf("|----------+----------+--------|\n");
}
