#pragma once

#include <string>
#include <map>
#include <vector>

#include "Cut.hh"

#include "TLorentzVector.h"
#include "TTree.h"

typedef std::map<std::string,TTree*> tree_collection;

void retrieve_values(tree_collection& forest, Long64_t entry);
void initialize_forest(tree_collection& forest);

template<typename T>
void setup_four_vector(TTree* tree, T& pt, T& eta, T& phi, T& E,const char* key, bool lo_case_E=true){
  char branch_name[50];
  snprintf(branch_name, 50,"%s_pt",key);
  tree->SetBranchAddress(branch_name, &pt);
  snprintf(branch_name, 50,"%s_eta",key);
  tree->SetBranchAddress(branch_name, &eta);
  snprintf(branch_name, 50,"%s_phi",key);
  tree->SetBranchAddress(branch_name, &phi);
  snprintf(branch_name, 50,lo_case_E ? "%s_e" : "%s_E",key);
  tree->SetBranchAddress(branch_name, &E);
}
template<typename T>
void setup_four_vector_output(TTree& tree, T& pt, T& eta, T& phi, T& E,const char* key){
  char branch_name[50];
  snprintf(branch_name, 50,"%s_pt",key);
  tree.Branch(branch_name, &pt);
  snprintf(branch_name, 50,"%s_eta",key);
  tree.Branch(branch_name, &eta);
  snprintf(branch_name, 50,"%s_phi",key);
  tree.Branch(branch_name, &phi);
  snprintf(branch_name, 50,"%s_e",key);
  tree.Branch(branch_name, &E);
}
double find_closest(const std::vector<double>& pt,
		    const std::vector<double>& eta,
		    const std::vector<double>& phi,
		    const std::vector<double>& E,
		    TLorentzVector& closest,
		    TLorentzVector& axis, size_t& idx);
int passed_trigger(std::vector<std::string>& trigger_names);
