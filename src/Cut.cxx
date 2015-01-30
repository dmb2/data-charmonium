#include "Cut.hh"
#include <cstdio>
#include <utility>
#include <algorithm>
#include <vector>
void cut_container::add_cut(cut<int> Cut, const std::string name){
  if(std::find(m_cutOrder.begin(),m_cutOrder.end(),name)==m_cutOrder.end()){
    m_cutOrder.push_back(name);
    m_cCutDefs[name]=Cut;
  }
  else{
    printf("Cut: %s already added to container!",name.c_str());
  }
}
void cut_container::add_cut(cut<double> Cut, const std::string name){
  if(std::find(m_cutOrder.begin(),m_cutOrder.end(),name)==m_cutOrder.end()){
    m_cutOrder.push_back(name);
    m_rCutDefs[name]=Cut;
  }
  else{
    printf("Cut: %s already added to container!",name.c_str());
  }
}
void cut_container::print_cut_table(){
  printf("|----------+----------+--------+-----------|\n");
  printf("| Cut Name | Cut Val  |  Count |  Weighted |\n");
  printf("|----------+----------+--------+-----------|\n");
  for(std::vector<std::string>::const_iterator CutName=m_cutOrder.begin();
      CutName!=m_cutOrder.end(); ++CutName){
    if(m_cCutDefs.find(*CutName)!=m_cCutDefs.end()){
      print_cut_summary(*CutName,m_cCutDefs[*CutName]);
    }
    else{
      if(m_rCutDefs.find(*CutName)!=m_rCutDefs.end()){
	print_cut_summary(*CutName,m_rCutDefs[*CutName]);
      }
    }
  }
  printf("|----------+----------+--------+-----------|\n");
}
void print_cut_summary(std::string CutName, cut<int> Cut){
  printf("| %-8s | %8d | %6d | %9.3g |\n",
	 CutName.c_str(), 
	 Cut.cut_value(),
	 Cut.count(),
	 Cut.weight());
}
void print_cut_summary(std::string CutName, cut<double> Cut){
  printf("| %-8s | %8.2g | %6d | %9.3g |\n",
	 CutName.c_str(),
	 Cut.cut_value(),
	 Cut.count(),
	 Cut.weight());
}
static bool sort_fn(std::pair<int,std::string> a, std::pair<int,std::string> b){
    return a.first > b.first;
}
void print_cut_table(real_cuts& CutDefReals,category_cuts& CutDefCats){
  std::vector<std::pair<int,std::string> > cut_keys;
  for(real_cuts::iterator cut=CutDefReals.begin();
      cut!=CutDefReals.end(); ++cut){
    cut_keys.push_back(std::pair<int,std::string>(cut->second.count(),cut->first));
  }
  for(category_cuts::iterator cut=CutDefCats.begin();
      cut!=CutDefCats.end(); ++cut){
    cut_keys.push_back(std::pair<int,std::string>(cut->second.count(),cut->first));
  }
  
  std::sort(cut_keys.begin(),cut_keys.end(),sort_fn);

  real_cuts::iterator rCutHandle;
  category_cuts::iterator cCutHandle;
  printf("|----------+----------+--------+-----------|\n");
  printf("| Cut Name | Cut Val  |  Count |  Weighted |\n");
  printf("|----------+----------+--------+-----------|\n");
  for(std::vector<std::pair<int,std::string> >::const_iterator cut_pair=cut_keys.begin();
      cut_pair != cut_keys.end(); ++cut_pair){
    const std::string& CutName = cut_pair->second;
    cCutHandle=CutDefCats.find(CutName);
    if(cCutHandle!=CutDefCats.end()){
      print_cut_summary(cCutHandle->first, cCutHandle->second);
    }
    else{
      rCutHandle=CutDefReals.find(CutName);
      if(rCutHandle!=CutDefReals.end()){
	print_cut_summary(rCutHandle->first,rCutHandle->second);
      }
    }
    
  }
  printf("|----------+----------+--------+-----------|\n");
}
