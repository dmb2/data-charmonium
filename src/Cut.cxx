#include "Cut.hh"
#include <cstdio>
#include <utility>
#include <algorithm>
#include <vector>
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
void cut_container::insert(std::string key, cut<int> cat_cut){
  std::vector<std::string>::const_iterator cut_name=std::find(insert_order.begin(),
							      insert_order.end(),key);
  if(cut_name==insert_order.end()){
    insert_order.push_back(key);
  }
  m_cat_cuts[key]=cat_cut;
};
void cut_container::insert(std::string key, cut<double> real_cut){
  std::vector<std::string>::const_iterator cut_name=std::find(insert_order.begin(),
							      insert_order.end(),key);
  if(cut_name==insert_order.end()){
    insert_order.push_back(key);
  }
  m_real_cuts[key]=real_cut;
};

void cut_container::print_cut_table(){
  for(std::vector<std::string>::const_iterator cn=insert_order.begin();
      cn!=insert_order.end(); ++cn){
    printf("%s\n",cn->c_str());
  }
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
